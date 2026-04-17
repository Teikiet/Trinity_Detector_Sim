#!/bin/bash
export LC_ALL=C LANG=C

DEBUG=1

# Containerized Trinity Simulation Chain
# Usage: $0 /in /out.txt

INPUT_DIR="$1"   # Should be /in (bound from host)
LOG_FILE="$2"    # Should be /out.txt (bound from host)
SEED="$3"        # Optional seed argument
ABSTABLE="$4"  # Optional abstable argument for CHASM, (0--default (m5_new), m5, m5_new, corsika, none)

# if not provided, set SEED and ABSTABLE to 0
SEED=${SEED:-0}
ABSTABLE=${ABSTABLE:-0}

SCRIPT_DIR="/root/TrinitySims/BatchSubmissionScripts/" # container path to scripts

chmod -R 777 "${INPUT_DIR}"
chmod 777 "$LOG_FILE"

# echo "Sourcing ROOT environment..."
# source /root/root-6.30.04/rinstall/bin/thisroot.sh


echo "which root: $(which root 2>/dev/null || true)"
echo "root-config --version: $(root-config --version 2>/dev/null || true)"
echo "root environment variables:" && env | grep -i root || true


# Redirect all output to log file
exec >> "$LOG_FILE" 2>&1

# input validation
# -----------------------------------------------------------------------------
if [[ -z "$INPUT_DIR" || -z "$LOG_FILE" ]]; then
    echo "Usage: $0 <input_directory> <log_file> [seed] [abstable]"
    exit 1
fi

# check for value of ABSTABLE, tell user if not valid -- options are 0, m5, m5_new, corsika
if [[ "$ABSTABLE" != "0" && "$ABSTABLE" != "m5" && "$ABSTABLE" != "m5_new" && "$ABSTABLE" != "corsika" && "$ABSTABLE" != 'none' ]]; then
    echo "RUNSCRIPT: Invalid ABSTABLE provided: $ABSTABLE. Valid options are 0, m5, m5_new, corsika, none."
    exit 1
fi

# check for value of ABSTABLE, tell user if not provided
if [[ "$ABSTABLE" == "0" ]]; then
    echo "RUNSCRIPT: No ABSTABLE provided, proceeding with default 'm5_new'"
else 
    echo "RUNSCRIPT: Using provided ABSTABLE: $ABSTABLE"
fi

# check for SEED value, tell user if not provided
if [[ "$SEED" == "0" ]]; then
    echo "RUNSCRIPT: No SEED provided, will use random seeds for each shower."
else
    echo "RUNSCRIPT: Using provided SEED: $SEED"
fi
# -----------------------------------------------------------------------------


echo "RUNSCRIPT: Starting simulation chain. Input: $INPUT_DIR, Log: $LOG_FILE"

# CHASM SECTION
# -----------------------------------------------------------------------------
chmod -R 777 "${INPUT_DIR}"    
${SCRIPT_DIR}/ShowerSimulationsCHASM.sh "$INPUT_DIR" "$INPUT_DIR/ISOG" ${SEED} ${ABSTABLE}
chmod 777 "$INPUT_DIR/CHASM/CHASM_Sim.in"
${SCRIPT_DIR}/executeCHASM.sh "$INPUT_DIR/CHASM/Input.txt"
# -----------------------------------------------------------------------------

# ISOG is small, skip compression

# CORSIKA IO READER SECTION
# -----------------------------------------------------------------------------
chmod -R 777 "${INPUT_DIR}"
${SCRIPT_DIR}/ShowerSimulationsCIOReader.sh "$INPUT_DIR" "$INPUT_DIR/CHASM" ${SEED}
wait
chmod -R 777 "${INPUT_DIR}"
${SCRIPT_DIR}/executeCIO.sh "$INPUT_DIR/CIO/CIO_Sim.in"
wait
# -----------------------------------------------------------------------------

# TEMP: DISABLE FOR TESTING SPEED
# Compress CHASM directory to save space
echo "Compressing CHASM directory..."
if [ -d "$INPUT_DIR/CHASM" ]; then
    tar -czf "$INPUT_DIR/CHASM.tar.gz" -C "$INPUT_DIR" CHASM && rm -rf "$INPUT_DIR/CHASM"
    echo "CHASM compression completed"
fi

# GROPTICS SECTION
# -----------------------------------------------------------------------------
chmod -R 777 "${INPUT_DIR}"
${SCRIPT_DIR}/ShowerSimulationsGrOptics.sh "$INPUT_DIR" "$INPUT_DIR/CIO" ${SEED}
chmod -R 777 "${INPUT_DIR}"

# execute GrOptics directly in this script
set -euo pipefail
exec </dev/null
set +o histexpand
unalias -a 2>/dev/null || true
IFS=$' \t\n'

INFILE="$INPUT_DIR/GROPT/GROP_Sim.in"

if [[ $DEBUG -ne 0 ]]; then
    echo "DEBUG mode enabled"
    # some debug stuff
    echo "HOST=$(hostname)  PWD=$(pwd)" # print some context info (DEBUG)
    echo "INFILE=$(readlink -f "$INFILE")" # full path to input file (DEBUG)
    first_cmd=$(head -n1 "$INFILE"); echo "FIRST_CMD: $first_cmd" # first command (DEBUG)
    cph=$(awk '{for(i=1;i<=NF;i++) if($i=="-if"){print $(i+1); exit}}' <<<"$first_cmd") # extract cio path from first command
    echo "CPH=$(readlink -f "$cph")" # full path to cio (DEBUG)
    stat -Lc 'size=%s perms=%a owner=%U group=%G' "$cph" # detailed info on cio (DEBUG)
    wc -l "$cph" # number of lines in cio -- should be many if has photons to print (DEBUG)
    head -n2 "$cph" | sed -n l # first two lines of cio (DEBUG)
    # end debug stuff
fi

[[ -s "$INFILE" ]] || { echo "Missing or empty $INFILE"; exit 2; } # check infile exists and not empty

# line by line:
# ignore blanks and comments; no login shell; no globbing side effects
# read each line, including last if no newline
# skip blank lines and comments
# remove trailing carriage return if present (e.g. from Windows)
# disable globbing
# split line into words, respecting quotes
# re-enable globbing
# print command (DEBUG)
# run command with no stdin
# wait for all background jobs to finish (if any)


while IFS= read -r line || [[ -n "$line" ]]; do 
  [[ -z "$line" || "${line:0:1}" == "#" ]] && continue 
  line=${line%$'\r'} 
  set -f 
  eval "set -- $line" 
  set +f 
  printf 'RUN:'; printf ' [%q]' "$@"; printf '\n' 
  "$@" </dev/null 
done < "$INFILE"
wait

# end of command execution

# -----------------------------------------------------------------------------

# Compress CIO directory to save space
echo "Compressing CIO directory..."
if [ -d "$INPUT_DIR/CIO" ]; then
    tar -czf "$INPUT_DIR/CIO.tar.gz" -C "$INPUT_DIR" CIO && rm -rf "$INPUT_DIR/CIO"
    echo "CIO compression completed"
fi

# CARE SECTION
# -----------------------------------------------------------------------------
chmod -R 777 "${INPUT_DIR}"
${SCRIPT_DIR}/ShowerSimulationsCARE.sh "$INPUT_DIR" "$INPUT_DIR/GROPT" "${SEED}"
wait
chmod -R 777 "${INPUT_DIR}"
${SCRIPT_DIR}/executeCARE.sh "$INPUT_DIR/CARE/CARE_Sim.in"
wait
# -----------------------------------------------------------------------------

# DISABLED FOR LEGACY ANALYSIS
# # Compress GROPT directory to save space
# echo "Compressing GROPT directory..."
# if [ -d "$INPUT_DIR/GROPT" ]; then
#     tar -czf "$INPUT_DIR/GROPT.tar.gz" -C "$INPUT_DIR" GROPT && rm -rf "$INPUT_DIR/GROPT"
#     echo "GROPT compression completed"
# fi 

# NOTE -- ROOT FILE MERGING DISABLED IN CURRENT VERSION, BUT IF WANTED IT SHOULD GO HERE

echo "Finished simulation chain for $(basename $INPUT_DIR)"