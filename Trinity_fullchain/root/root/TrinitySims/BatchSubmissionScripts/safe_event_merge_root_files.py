#!/usr/bin/env python3
"""
Safe ROOT file merger that creates separate directories for each event.
Uses entry-by-entry copying instead of tree cloning to avoid corruption.

Usage: python safe_event_merge_root_files.py <BASEDIR>
Where BASEDIR is the timestamp directory (e.g., /path/to/output/20250805_123456)
"""

import os
import sys
import glob
import argparse
import shutil
import tarfile
from pathlib import Path

try:
    import ROOT
    # Set ROOT to batch mode to avoid GUI issues
    ROOT.gROOT.SetBatch(True)
    # Disable ROOT info messages to reduce clutter
    ROOT.gErrorIgnoreLevel = ROOT.kWarning
except ImportError:
    print("Error: PyROOT not available. Make sure ROOT is properly installed with Python bindings.")
    sys.exit(1)

def validate_root_file(file_path):
    """
    Validate that a ROOT file can be opened and read.
    """
    try:
        test_file = ROOT.TFile.Open(file_path, "READ")
        if not test_file or test_file.IsZombie():
            return False
        
        # Try to read the keys
        keys = test_file.GetListOfKeys()
        if keys.GetSize() == 0:
            test_file.Close()
            return False
        
        test_file.Close()
        return True
    except Exception as e:
        print(f"  Error validating {file_path}: {e}")
        return False

def copy_tree_structure(source_tree, target_dir, event_name):
    """
    Copy tree structure and data without using Clone().
    Creates a new tree and copies branches one by one.
    """
    try:
        # Change to the target directory
        target_dir.cd()
        
        # Create new trees for this event
        trees_created = {}
        
        # Get the source tree name
        tree_name = source_tree.GetName()
        tree_title = source_tree.GetTitle()
        
        # print(f"      Creating tree: {tree_name}")
        
        # Create new tree
        new_tree = ROOT.TTree(tree_name, tree_title)
        
        # Get list of branches from source
        branch_list = source_tree.GetListOfBranches()
        branch_info = {}
        
        # Set up branches in the new tree
        for branch in branch_list:
            branch_name = branch.GetName()
            branch_type = branch.GetClassName()
            leaf = branch.GetLeaf(branch_name)
            leaf_type = leaf.GetTypeName() if leaf else ""
            
            # print(f"        Setting up branch: {branch_name} (class: {branch_type}, leaf: {leaf_type})")
            
            # Create variables to hold the data based on the actual leaf type
            if 'vector' in branch_type.lower():
                # Handle vector branches
                if 'int' in branch_type.lower():
                    var = ROOT.std.vector('int')()
                elif 'float' in branch_type.lower():
                    var = ROOT.std.vector('float')()
                elif 'double' in branch_type.lower():
                    var = ROOT.std.vector('double')()
                else:
                    # print(f"          Warning: Unknown vector type {branch_type}")
                    continue
            else:
                # Handle simple types based on leaf type
                if leaf_type == 'Int_t' or '/I' in str(leaf.GetTitle() if leaf else ''):
                    var = ROOT.std.array('int', 1)()
                elif leaf_type == 'Float_t' or '/F' in str(leaf.GetTitle() if leaf else ''):
                    var = ROOT.std.array('float', 1)()
                elif leaf_type == 'Double_t' or '/D' in str(leaf.GetTitle() if leaf else ''):
                    var = ROOT.std.array('double', 1)()
                elif 'string' in branch_type.lower() or leaf_type == 'string':
                    var = ROOT.std.string()
                else:
                    # Default to double for unknown types
                    # print(f"          Warning: Unknown type {branch_type}/{leaf_type}, defaulting to double")
                    var = ROOT.std.array('double', 1)()
            
            branch_info[branch_name] = var
            
            # Set branch address in new tree
            if 'vector' in branch_type.lower():
                new_tree.Branch(branch_name, var)
            else:
                # Use the proper type suffix based on the leaf type
                if leaf and leaf.GetTitle():
                    type_suffix = leaf.GetTitle().split('/')[-1] if '/' in leaf.GetTitle() else 'D'
                else:
                    type_suffix = 'D'  # Default to double
                new_tree.Branch(branch_name, var, f"{branch_name}/{type_suffix}")
        
        # Copy data entry by entry
        n_entries = source_tree.GetEntries()
        # print(f"        Copying {n_entries} entries")

        for entry in range(n_entries):
            source_tree.GetEntry(entry)
            
            # Copy data from source to target variables
            for branch_name, var in branch_info.items():
                try:
                    source_data = getattr(source_tree, branch_name)
                    
                    if hasattr(var, 'clear'):  # Vector type
                        var.clear()
                        for item in source_data:
                            var.push_back(item)
                    elif hasattr(var, 'assign'):  # String type
                        var.assign(str(source_data))
                    else:  # Simple type (array)
                        # Convert to the appropriate type
                        if 'int' in str(type(var[0])):
                            var[0] = int(source_data)
                        elif 'float' in str(type(var[0])):
                            var[0] = float(source_data)
                        else:
                            var[0] = source_data
                        
                except Exception as e:
                    # print(f"          Warning: Could not copy {branch_name}: {e}")
                    # Try to set a default value
                    try:
                        if hasattr(var, 'clear'):  # Vector
                            var.clear()
                        elif hasattr(var, 'assign'):  # String
                            var.assign("")
                        else:  # Simple type
                            var[0] = 0
                    except:
                        pass
            
            # Fill the new tree
            new_tree.Fill()
        
        # Write the tree
        new_tree.Write()
        #print(f"         Tree {tree_name} written with {new_tree.GetEntries()} entries")
        
        return True
        
    except Exception as e:
        print(f"         Error copying tree: {e}")
        return False

def merge_root_files_by_event(base_dir):
    """
    Safely merge ROOT files with each file becoming a separate event directory.
    Uses TTree::CloneTree to avoid data corruption from manual buffer/type guessing.
    """
    import os, glob, shutil, tarfile
    import ROOT

    # --- helper: lossless tree copy into a directory ---
    def _clone_tree_into_dir(source_tree, target_dir):
        """
        Deep-copy a TTree into target_dir using CloneTree.
        Preserves exact branch types/streamers/baskets.
        """
        target_dir.cd()
        cloned = source_tree.CloneTree(-1, "fast")  # copy all entries
        cloned.SetDirectory(target_dir)
        n = cloned.GetEntries()
        # Ensure metadata and baskets are written under the target directory
        cloned.Write("", ROOT.TObject.kOverwrite)
        return n

    # Construct path to the GROPT/Tilt_91.560000 directory
    gropt_dir = os.path.join(base_dir, "GROPT", "Tilt_91.560000")

    if not os.path.exists(gropt_dir):
        print(f"Error: Directory {gropt_dir} does not exist")
        return False

    # Find all ROOT files in the directory
    root_files = glob.glob(os.path.join(gropt_dir, "*.root"))

    # Filter out any existing merged files
    root_files = [f for f in root_files if not os.path.basename(f).startswith("merged_")]

    if not root_files:
        print(f"No ROOT files found in {gropt_dir}")
        return False

    print(f"Found {len(root_files)} ROOT files to merge")

    # Validate all input files first
    print("Validating input files...")
    valid_files = []
    for root_file in sorted(root_files):
        # print(f"  Checking {os.path.basename(root_file)}...")
        if validate_root_file(root_file):
            #print(" Valid")
            valid_files.append(root_file)
        else:
            print(f" Invalid {os.path.basename(root_file)} - skipping")

    if not valid_files:
        print("Error: No valid ROOT files found")
        return False

    print(f"Will merge {len(valid_files)} valid files")

    # Create output filename
    output_file = os.path.join(gropt_dir, "merged_simulation_output.root")
    temp_output_file = output_file + ".tmp"

    # Create the output ROOT file
    try:
        output_root = ROOT.TFile(temp_output_file, "RECREATE")

        if not output_root or not output_root.IsOpen():
            print(f"Error: Could not create temporary output file {temp_output_file}")
            return False

        print(f"Creating merged ROOT file: {output_file}")

        # Process each ROOT file
        processed_files = []

        for i, root_file_path in enumerate(valid_files):
            filename = os.path.basename(root_file_path)
            #print(f"Processing {filename} ({i+1}/{len(valid_files)})")

            # Open the input file
            input_file = ROOT.TFile.Open(root_file_path, "READ")

            if not input_file or not input_file.IsOpen():
                print(f"Warning: Could not open {root_file_path}, skipping")
                continue

            # Get all keys from the input file
            keys = input_file.GetListOfKeys()

            if not keys or keys.GetSize() == 0:
                print(f"Warning: No objects found in {filename}, skipping")
                input_file.Close()
                continue

            # Create a directory in the output file for this event
            # Use filename without extension as directory name
            dir_name = os.path.splitext(filename)[0]
            output_dir = output_root.mkdir(dir_name)

            if not output_dir:
                print(f"Error: Could not create directory {dir_name} in output file")
                input_file.Close()
                continue

            objects_copied = 0
            for key in keys:
                obj_name = key.GetName()
                obj_class = key.GetClassName()
                obj = input_file.Get(obj_name)

                if not obj:
                    print(f"         Could not retrieve {obj_name}")
                    continue

                #print(f"      Processing {obj_name} ({obj_class})")

                try:
                    if obj.InheritsFrom("TTree"):
                        # Lossless copy of the tree and all entries into this event directory
                        n_entries = _clone_tree_into_dir(obj, output_dir)
                        #print(f"         Cloned tree {obj.GetName()} with {n_entries} entries")
                        objects_copied += 1
                    else:
                        # Copy other objects (histos, THn, TGraphs, etc.) as-is
                        output_dir.cd()
                        # Prefer direct Write to keep original object state; Clone() can be used too
                        bytes_written = obj.Write(obj.GetName(), ROOT.TObject.kOverwrite)
                        if bytes_written > 0:
                            objects_copied += 1
                            #print(f"         Copied {obj_name} - {bytes_written} bytes")
                        else:
                            print(f"         Failed to write {obj_name}")
                except Exception as e:
                    print(f"         Error copying {obj_name}: {e}")

            input_file.Close()

            if objects_copied > 0:
                processed_files.append(root_file_path)
                # print(f"     Successfully processed {objects_copied} objects")
            else:
                print(f"    Warning: No objects successfully copied from {filename}")

            # Force write after each file
            output_root.Write("", ROOT.TObject.kOverwrite)

        # Final write and close
        print("Finalizing merged file...")
        output_root.Write("", ROOT.TObject.kOverwrite)
        output_root.Close()

        # Move temporary file to final location
        shutil.move(temp_output_file, output_file)

        if processed_files:
            print(f"Successfully merged {len(processed_files)} ROOT files into {output_file}")

            # Create backup directory for original files
            backup_dir = os.path.join(gropt_dir, "original_root_files_backup")
            os.makedirs(backup_dir, exist_ok=True)

            print(f"\nMoving original ROOT files to backup directory: {backup_dir}")
            for root_file_path in processed_files:
                try:
                    backup_path = os.path.join(backup_dir, os.path.basename(root_file_path))
                    shutil.move(root_file_path, backup_path)
                    #print(f"    Moved {os.path.basename(root_file_path)} to backup")
                except OSError as e:
                    print(f"    Warning: Could not move {root_file_path}: {e}")

            # Compress the backup directory
            print("Compressing backup directory...")
            backup_archive = os.path.join(gropt_dir, "original_root_files_backup.tar.gz")
            try:
                with tarfile.open(backup_archive, "w:gz") as tar:
                    tar.add(backup_dir, arcname=os.path.basename(backup_dir))

                # Remove the uncompressed backup directory
                shutil.rmtree(backup_dir)
                print(f" Backup compressed to: {backup_archive}")
            except Exception as e:
                print(f"Warning: Could not compress backup directory: {e}")
                print(f"Backup files remain in: {backup_dir}")

            print("Merge operation completed successfully!")
            return True
        else:
            print("Error: No files were successfully processed")
            return False

    except Exception as e:
        print(f"Error during merge process: {e}")
        # Best-effort cleanup of temp file if it exists and output wasn't moved
        try:
            if os.path.exists(temp_output_file):
                os.remove(temp_output_file)
        except Exception:
            pass
        return False

def main():
    parser = argparse.ArgumentParser(
        description="Safely merge ROOT files with event-by-event structure",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Example usage:
  python safe_event_merge_root_files.py /home/edenlinux/TrinitySims/output/20250806_171922_ers

This script:
- Creates separate directories for each input ROOT file (event)
- Copies tree data entry-by-entry to avoid corruption
- Validates all input files before merging
- Backs up original files and compresses them
- Provides detailed progress feedback
        """
    )
    
    parser.add_argument(
        "basedir",
        help="Path to the timestamp directory containing GROPT folder"
    )
    
    args = parser.parse_args()
    
    # Validate input directory
    if not os.path.exists(args.basedir):
        print(f"Error: Directory {args.basedir} does not exist")
        sys.exit(1)
    
    if not os.path.isdir(args.basedir):
        print(f"Error: {args.basedir} is not a directory")
        sys.exit(1)
    
    # Run the merge operation
    success = merge_root_files_by_event(args.basedir)
    
    if not success:
        sys.exit(1)

if __name__ == "__main__":
    main()
