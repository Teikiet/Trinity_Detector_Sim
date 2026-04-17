#!/bin/bash

# Script to run Trinity simulations via apptainer shell with proper ROOT sourcing
# Usage: ./runscript.sh <sif_file> <sim_directory> <output_file>

# Check if correct number of arguments provided
if [ $# -ne 2 ]; then
    echo "Usage: $0 <sif_file> <sim_directory> <output_file>"
    echo "Example: $0 TrinitySims.sif /home/user/simdata/input /home/user/simdata/output.txt"
    exit 1
fi

# old standard: 

# SIMDIR="$1"      # e.g. /path/DATE_ID_INITIALS
# LOGFILE="$2"     # e.g. /path/out_DATE_ID_INITIALS.txt
# IMAGE="$3"       # e.g. /path/TrinitySims.sif


# Assign arguments to variables

SIM_DIR="$1"
OUTPUT_FILE="$2"
SIF_FILE="/storage/osg-otte1/nschapera6/containers/TrinitySims_manual.sif" # this is such a hack. apparently the sif file submission or something is PATH DEPENDENT. 
# just like... dont touch it. it has to be in this PARTICULAR directory for it to work. I have no fucking clue why.
# god save our souls.
cd /storage/osg-otte1/nschapera6/containers/
# Check if SIF file exists
if [ ! -f "$SIF_FILE" ]; then
    echo "Error: SIF file '$SIF_FILE' not found!"
    exit 1
fi

# Check if simulation directory exists
if [ ! -d "$SIM_DIR" ]; then
    echo "Error: Simulation directory '$SIM_DIR' not found!"
    exit 1
fi

# Create output directory if it doesn't exist
OUTPUT_DIR=$(dirname "$OUTPUT_FILE")
if [ ! -d "$OUTPUT_DIR" ]; then
    echo "Creating output directory: $OUTPUT_DIR"
    mkdir -p "$OUTPUT_DIR"
fi

echo "Starting Trinity simulation..."
echo "SIF file: $SIF_FILE"
echo "Simulation directory: $SIM_DIR"
echo "Output file: $OUTPUT_FILE"
echo "---"
pwd
echo "---"
# Use a here-document to pass commands to apptainer shell
apptainer shell --bind "$SIM_DIR:/in" --bind "$OUTPUT_FILE:/out.txt" "$SIF_FILE" << 'EOF'
echo "Sourcing ROOT environment..."
source /root/root-6.30.04/rinstall/bin/thisroot.sh


echo "which root: $(which root 2>/dev/null || true)"
echo "root-config --version: $(root-config --version 2>/dev/null || true)"
echo "root environment variables:" && env | grep -i root || true


echo "ROOT sourced successfully"
echo "Running Trinity simulation..."
bash /root/TrinitySims/BatchSubmissionScripts/Hive_SequentialChainJobPerShower/runTrinityShowersMCSequentialChain.sh /in /out.txt
echo "Simulation completed"
exit
EOF

echo "---"
echo "Script execution completed!"
