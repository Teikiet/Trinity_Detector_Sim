#!/bin/bash

usage() {
  echo "Usage: $0 -d BASE_DATE -i INITIALS [-o OUTPUT_DIR] [-m MAX_JOBS] [-s SLURM_OUTPUT_DIR] [-k]"
  echo "  -d BASE_DATE       Date prefix for folders, in YYYYMMDD format"
  echo "  -i INITIALS        Your initials (e.g. JSM)"
  echo "  -o OUTPUT_DIR      Directory to search for subdirectories (default: /storage/hive/project/phy-otte/shared/Trinity/Simulations/TDemSims)"
  echo "  -m MAX_JOBS        Maximum number of jobs to run in parallel (default: 500)"
  echo "  -s SLURM_OUTPUT_DIR Directory for SLURM output files (default: same as OUTPUT_DIR)"
  echo "  -k                 Skip already processed directories (only check for .root files in Tilt directory if this flag is set)"
  echo "Example: $0 -d 20231001 -i JSM"
  echo "Example with custom output dir: $0 -d 20231001 -i JSM -o /path/to/custom/dir"
  echo "Example with max jobs: $0 -d 20231001 -i JSM -m 300"
  echo "Example with SLURM output dir: $0 -d 20231001 -i JSM -s /path/to/slurm/logs"
  echo "Example with skip: $0 -d 20231001 -i JSM -k"
  exit 1
}

# Parse command line options

SKIP_PROCESSED=0
while getopts ":d:i:o:m:s:k" opt; do
  case $opt in
    m) MAX_JOBS="$OPTARG" ;;
    d) BASE_DATE="$OPTARG" ;;
    i) INITIALS="$OPTARG" ;;
    o) OUTPUT_DIR="$OPTARG" ;;
    s) SLURM_OUTPUT_DIR="$OPTARG" ;;
    k) SKIP_PROCESSED=1 ;;
    \?) echo "Invalid option: -$OPTARG" >&2; usage ;;
    :)  echo "Option -$OPTARG requires an argument." >&2; usage ;;
  esac
done

# make sure we got both
if [[ -z "$BASE_DATE" || -z "$INITIALS" ]]; then
  usage
fi

# Configuration - set defaults if not provided
if [[ -z "$OUTPUT_DIR" ]]; then
  OUTPUT_DIR="/storage/hive/project/phy-otte/shared/Trinity/Simulations/TDemSims"
fi
if [[ -z "$SLURM_OUTPUT_DIR" ]]; then
  SLURM_OUTPUT_DIR="$OUTPUT_DIR"
fi
if [[ -z "$MAX_JOBS" ]]; then
  MAX_JOBS=500
fi
SUBMIT_SCRIPT="/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/BatchSubmissionScripts/Hive_SequentialChainJobPerShower/runTrinityShowersMCSequentialChain.slurm.sh"

# throttle settings
CHECK_INTERVAL=120     # seconds to wait before re-checking

# loop over only directories matching the given date and initials
for DIR in "${OUTPUT_DIR}/${BASE_DATE}"_??????_"${INITIALS}"; do
  [[ ! -d "$DIR" ]] && continue
  TIMESTAMP=$(basename "$DIR")

  # skip if already processed (check for .root files in Tilt directory) only if -k/--skip is set
  if [[ $SKIP_PROCESSED -eq 1 ]]; then
    TILT_DIR="${DIR}/GROPT/Tilt_91.560000"
    if [[ -d "$TILT_DIR" ]] && [[ $(find "$TILT_DIR" -maxdepth 1 -name "*.root" | wc -l) -gt 0 ]]; then
      echo "Skipping ${TIMESTAMP}: already has .root files in Tilt directory."
      continue
    fi
  fi

  # throttle: wait until you have fewer than $MAX_JOBS jobs pending or running
  while true; do
    CURRENT=$(squeue -u "$USER" -h | wc -l)
    if (( CURRENT < MAX_JOBS )); then
      break
    fi
    echo "You have $CURRENT jobs (>= $MAX_JOBS), sleeping for $CHECK_INTERVAL s…"
    sleep "$CHECK_INTERVAL"
  done

  # submit the job
  JOB_ID=$(sbatch --parsable "$SUBMIT_SCRIPT" "$TIMESTAMP")
  echo "Submitted job ${JOB_ID} for folder ${TIMESTAMP}"

  # small pause to avoid hammering sbatch
  # sleep 1
done
