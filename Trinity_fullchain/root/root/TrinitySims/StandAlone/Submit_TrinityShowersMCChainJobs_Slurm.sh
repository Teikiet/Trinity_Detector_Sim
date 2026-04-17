#!/bin/bash

OUTPUT_DIR="/storage/hive/project/phy-otte/shared/Trinity/Simulations/TDemSims"
SUBMIT_SCRIPT="/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/BatchSubmissionScripts/Hive_SequentialChainJobPerShower/runTrinityShowersMCSequentialChain.slurm.sh"

# Loop over all folders matching the yyyymmdd_hhmmss pattern
for DIR in "${OUTPUT_DIR}"/20??????_??????; do
    if [[ -d "$DIR" ]]; then
        TIMESTAMP=$(basename "$DIR")

         # Optional: skip folders that already have finished output
        if [[ -f "${DIR}/GROPT/GROP_Sim.in" ]]; then
            echo "Skipping ${TIMESTAMP}: already processed."
            continue
        fi

        # Submit job
        JOB_ID=$(sbatch --parsable "$SUBMIT_SCRIPT" "$TIMESTAMP")
        echo "Submitted job ${JOB_ID} for folder ${TIMESTAMP}"

        # Optional: wait to throttle job submissions
        sleep 2
    fi
done
