#!/bin/bash
#SBATCH -A hive-aotte6
#SBATCH -N 1
#SBATCH --ntasks=10
#SBATCH --mem-per-cpu=2G
#SBATCH -t 15:00:00
#SBATCH -p hive
#SBATCH -J TrinitySimChain_${1}

# Your commands here, e.g.:
echo "Running job with timestamp: $1"


module load root

# Input: folder name like "20250617_153012"
TIMESTAMP=$1
if [[ -z "$TIMESTAMP" ]]; then
    echo "Usage: $0 <yyyymmdd_hhmmss>"
    exit 1
fi

SCRIPT_DIR="/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/StandAlone/"
BASE_DIR="/storage/hive/project/phy-otte/shared/Trinity/Simulations/TDemSims/${TIMESTAMP}"
OUTFILE="/storage/hive/project/phy-otte/shared/Trinity/Simulations/TDemSims/out_${TIMESTAMP}.txt"

exec >> "$OUTFILE" 2>&1

echo "Starting simulation for ${TIMESTAMP}"

${SCRIPT_DIR}/ShowerSimulationsCHASM.sh /storage/hive/project/phy-otte/shared/Trinity/Simulations/TDemSims "${BASE_DIR}/ISOG" ${TIMESTAMP} 1

chmod 777 "${BASE_DIR}/CHASM/CHASM_Sim.in"
${SCRIPT_DIR}/executeCHASM.sh "${BASE_DIR}/CHASM/Input.txt"

${SCRIPT_DIR}/ShowerSimulationsCIOReader.sh "${BASE_DIR}" "${BASE_DIR}/CHASM"
${SCRIPT_DIR}/executeCIO.sh "${BASE_DIR}/CIO/CIO_Sim.in"

${SCRIPT_DIR}/ShowerSimulationsGrOptics.sh "${BASE_DIR}" "${BASE_DIR}/CIO" 1
${SCRIPT_DIR}/executeGRO.sh "${BASE_DIR}/GROPT/GROP_Sim.in"

echo "Finished simulation for ${TIMESTAMP}"
