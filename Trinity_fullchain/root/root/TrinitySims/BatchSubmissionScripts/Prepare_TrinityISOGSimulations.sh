#!/bin/bash

# Number of ISOG simulations to prepare
N_JOBS=100

# Base output directory
OUTPUT_DIR="/root/TrinitySims/output"

# Simulation name prefix (for input files, if needed)
SIM_NAME="myRun"

for ((i=1; i<=N_JOBS; i++)); do
    TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
    FOLDER="${OUTPUT_DIR}/${TIMESTAMP}"

    {
        echo "Creating simulation setup for ${FOLDER}"

        ./ShowerSimulationsISOG.sh "$OUTPUT_DIR" "$SIM_NAME" "$TIMESTAMP"

        ./executeISOG.sh "${FOLDER}/ISOG/ISOG_Sim.in"

    } > "${OUTPUT_DIR}/out_${TIMESTAMP}.txt" 2>&1

    sleep 1
done
