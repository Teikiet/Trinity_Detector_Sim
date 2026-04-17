#!/bin/bash

#SBATCH --account=owner-guest
#SBATCH --partition=kingspeak-guest
#SBATCH --time=4:00:00
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --mem=16G
#SBATCH --job-name=trinity_chain
#SBATCH --output=log/trinity_chain.out
#SBATCH --error=log/trinity_chain.err

# Usage: sbatch run_script.slurm <sim_directory> <output_file_path>

if [ $# -ne 2 ]; then
    echo "Usage: sbatch $0 <sim_directory> <output_file_path>"
    exit 1
fi

SIM_DIR_HOST="$1"
OUT_FILE_HOST="$2"
SANDBOX_DIR="/uufs/chpc.utah.edu/common/home/u1520754/Detector_Sims/Trinity_fullchain/root"
#remove outfile if exists
if [ -f "$OUT_FILE_HOST" ]; then
    rm "$OUT_FILE_HOST"
fi
# Setup Output Logic
OUT_DIR_HOST=$(dirname "$OUT_FILE_HOST")
OUT_FILENAME=$(basename "$OUT_FILE_HOST")

# Create directories
mkdir -p "$OUT_DIR_HOST"
mkdir -p log

# --- FIX FOR CHMOD ERROR ---
# Create the empty file and make it writable BEFORE the container starts
# so the internal script finds it when it runs 'chmod'
touch "$OUT_FILE_HOST"
chmod 666 "$OUT_FILE_HOST"
# ---------------------------

module load apptainer

echo "Running Trinity simulation..."

# We bind the Host Output Dir to /output_mnt
# We tell the internal script to write to /output_mnt/filename.txt

apptainer exec --pid --cleanenv \
    --bind "$SIM_DIR_HOST:/in" \
    --bind "$OUT_DIR_HOST:/output_mnt" \
    "$SANDBOX_DIR" \
    bash /root/TrinitySims/BatchSubmissionScripts/Hive_SequentialChainJobPerShower/runTrinityShowersMCSequentialChain_2.sh \
    "/in" \
    "/output_mnt/$OUT_FILENAME" \
    "${RANDOM}" \
    "m5_new"


EXIT_CODE=$?
echo "Exit Code: $EXIT_CODE"
exit $EXIT_CODE
