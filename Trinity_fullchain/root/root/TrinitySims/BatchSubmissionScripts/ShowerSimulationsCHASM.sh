#!/bin/bash

CHASM=/root/TrinitySims/CHASM_NuSpacesim/demo/RunCHASM.py
EXECUTESCRIPT=/root/TrinitySims/BatchSubmissionScripts/executeCHASM.sh
INPUTMASK=Input.txt

FINALDIRBASE=$(realpath "$1")   # Base directory for outputs (e.g. /in)
INPUTDIR=$(realpath "$2")       # Directory containing Tilt* subdirs (e.g. /in/ISOG)
USESEED=$3
ABSTABLE=$4

MAXTASKS=100
MAXJOBS=400

#!/bin/bash

# Place CHASM output under the base directory, not inside the input (Tilt) directory.
# This changes previous behavior where CHASM lived in $INPUTDIR/CHASM (e.g. /in/ISOG/CHASM)
# Now it will be at $FINALDIRBASE/CHASM (e.g. /in/CHASM) per user request.
FINALDIR=${FINALDIRBASE}/CHASM
mkdir -p ${FINALDIR}

# Ensure output files exist so redirects don’t fail later
: > "${FINALDIR}/CHASM_Sim.in"
: > "${FINALDIR}/GrOpticsTokens.txt"

filecount=0
linecount=0
shwrcount=0
ARGUMENTSGRO=""
RANDFLAG=true
ABSTABLE=${ABSTABLE:-0}
USESEED=${USESEED:-0}

# Use fixed seed if provided; otherwise random per event
if [ "${USESEED}" != "0" ]; then
  echo "Using provided CHASM seed: ${USESEED}"
  SEED="${USESEED}"
  RANDFLAG=false
else
  echo "Will use random seeds for each CHASM run."
fi

if [ "${ABSTABLE}" != "0" ]; then
  echo "Using provided ABSTABLE: ${ABSTABLE}"
else
  echo "No ABSTABLE provided, proceeding with default 'm5_new'"
  ABSTABLE="m5_new"
fi

echo "Starting directory loop, looking for directories matching: ${INPUTDIR}/Tilt*/"
echo "Using CHASM Abstable: ${ABSTABLE}"

for FLDR in "${INPUTDIR}"/Tilt*/; do
  LAST_DIR=$(basename "${FLDR}")
  echo "${LAST_DIR}"
  FINALDIRSH=${FINALDIR}/${LAST_DIR}/
  mkdir -p "${FINALDIRSH}"

  if [ -d "${FLDR}" ]; then

        for FL in "${FLDR}"/*txt; do
      BASENAME=$(basename "${FL}")

      # Read all lines at once (handles missing trailing newline)
      mapfile -t lines < "${FL}"

      # Skip the first two header lines, process from line index 2
      for (( idx=0; idx<${#lines[@]}; idx++ )); do
        line="${lines[idx]}"

        if (( idx < 2 )); then
          echo "Skipping header line $((idx+1))"
          continue
        fi

        # Trim CR in case of Windows line endings
        line="${line%$'\r'}"
        [[ -z "$line" ]] && continue

        # tokenize and require at least 11 CHASM args
        # shellcheck disable=SC2206
        tokens=($line)
        if [ ${#tokens[@]} -lt 11 ]; then
          echo "Skipping short line (found ${#tokens[@]} tokens): $line"
          continue
        fi

        ARGUMENTS=""
        for (( i=0; i<11; i++ )); do
          ARGUMENTS="${ARGUMENTS} ${tokens[i]}"
        done

        # choose seed per event if random mode
        if [ "${RANDFLAG}" = true ]; then
          SEED=$RANDOM
        fi

        FINALDIRSH=${FINALDIR}/${LAST_DIR}/
        mkdir -p "${FINALDIRSH}"

        # Append one CHASM command line
        echo "python3.10 ${CHASM} ${ARGUMENTS} ${FINALDIRSH}/${BASENAME%.*}_$((shwrcount)).dat --seed ${SEED} --abstable ${ABSTABLE}" >> "${FINALDIR}/CHASM_Sim.in"
        echo "${ARGUMENTSGRO}" >> "${FINALDIR}/GrOpticsTokens.txt"
        ((shwrcount++))
      done

      ((filecount++))
      echo "File: ${filecount}"
    done
  fi
done

# Write the path list for the executor
echo "${FINALDIR}/CHASM_Sim.in" > "${FINALDIR}/${INPUTMASK}"

echo "CHASM input file prepared: ${FINALDIR}/CHASM_Sim.in"
