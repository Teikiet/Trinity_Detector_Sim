#!/usr/bin/env python3
"""
Script to analyze CORSIKA files using multiple approaches
"""

import os
import struct
import numpy as np
import matplotlib.pyplot as plt

def read_corsika_manual(filename):
    """
    Manually read CORSIKA file structure
    """
    print(f"\n=== Manual CORSIKA file analysis: {filename} ===")
    
    with open(filename, 'rb') as f:
        # CORSIKA uses FORTRAN binary format with record markers
        # Each record starts with a 4-byte length, then data, then 4-byte length again
        
        # Read first record marker
        record_len_bytes = f.read(4)
        if len(record_len_bytes) != 4:
            print("Error: Could not read record length")
            return
            
        record_len = struct.unpack('<I', record_len_bytes)[0]  # Little-endian unsigned int
        print(f"First record length: {record_len} bytes")
        
        # Read the record data
        record_data = f.read(record_len)
        print(f"Actually read: {len(record_data)} bytes")
        
        # Check for RUNH marker
        if b'RUNH' in record_data:
            runh_pos = record_data.find(b'RUNH')
            print(f"Found RUNH at position {runh_pos} in record")
            
            # RUNH is followed by run header data (273 floats total, but first few are important)
            # Skip the 'RUNH' string and read some header values
            header_start = runh_pos + 4
            if len(record_data) >= header_start + 40:  # Ensure we have enough data
                # Read some key values (all are 4-byte floats in CORSIKA)
                header_floats = struct.unpack('<10f', record_data[header_start:header_start+40])
                print(f"Run number: {header_floats[0]}")
                print(f"Date: {header_floats[1]}")
                print(f"CORSIKA version: {header_floats[2]}")
                print(f"Observation level: {header_floats[3]} cm")
                print(f"Particle ID: {header_floats[4]}")
                print(f"Energy range: {header_floats[5]} - {header_floats[6]} GeV")
        
        # Look for event headers (EVTH)
        f.seek(0)
        data_chunk = f.read(10000)  # Read larger chunk to look for EVTH
        if b'EVTH' in data_chunk:
            evth_pos = data_chunk.find(b'EVTH')
            print(f"Found EVTH (Event Header) at position {evth_pos}")
        else:
            print("No EVTH found in first 10KB - might be a run with no events")

def try_corsikaio_alternatives(filename):
    """
    Try different corsikaio approaches
    """
    print(f"\n=== Trying corsikaio alternatives ===")
    
    try:
        # Try with explicit parameters
        from corsikaio import CorsikaCherenkovFile
        
        # Method 1: Try with different read modes
        try:
            print("Trying with default parameters...")
            with CorsikaCherenkovFile(filename) as f:
                print("Success with default parameters!")
                print(f"Run number: {f.run_header.get('run_number', 'N/A')}")
                return True
        except Exception as e:
            print(f"Failed with default parameters: {e}")
        
        # Method 2: Try with mmapping disabled
        try:
            print("Trying with mmap=False...")
            with CorsikaCherenkovFile(filename, mmap=False) as f:
                print("Success with mmap=False!")
                print(f"Run number: {f.run_header.get('run_number', 'N/A')}")
                return True
        except Exception as e:
            print(f"Failed with mmap=False: {e}")
            
        # Method 3: Try reading as raw eventio
        try:
            print("Trying with raw eventio...")
            import eventio
            with eventio.EventIOFile(filename) as f:
                print("Successfully opened with eventio!")
                for i, obj in enumerate(f):
                    print(f"Object {i}: {type(obj)}")
                    if i > 5:  # Don't read too many
                        break
                return True
        except Exception as e:
            print(f"Failed with eventio: {e}")
            
    except ImportError as e:
        print(f"Import error: {e}")
    
    return False

def main():
    filename = 'testfile.dat'
    
    if not os.path.exists(filename):
        print(f"File {filename} not found!")
        return
    
    print(f"File: {filename}")
    print(f"Size: {os.path.getsize(filename):,} bytes")
    
    # Try manual reading first
    read_corsika_manual(filename)
    
    # Try corsikaio alternatives
    if not try_corsikaio_alternatives(filename):
        print("\nAll corsikaio methods failed. The file might be:")
        print("1. In a different CORSIKA format than expected")
        print("2. Corrupted")
        print("3. Using a CORSIKA version not supported by this corsikaio version")
        print("4. Missing required headers or having non-standard structure")
        
        print("\nRecommendation: Use the corsikaIOreader binary instead:")
        print("./corsikaIOreader -cors testfile.dat -grisu output.grisu")

if __name__ == "__main__":
    main()