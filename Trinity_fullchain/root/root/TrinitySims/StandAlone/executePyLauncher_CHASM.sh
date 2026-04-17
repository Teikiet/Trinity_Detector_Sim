#!/bin/bash
#SBATCH -A hive-aotte6
#SBATCH -JCHASMSimDiffuse
#SBATCH -n200
#SBATCH --mem-per-cpu=8G
#SBATCH --time=2-0:0
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=omatamala3@gatech.edu
#SBATCH -phive-nvme-sas

INFILE=$(head -n 1 $1)

module load anaconda3
module load pylauncher/4.0
module load gcc/8.3.0-rv4av7
module load gsl
module load root

ROBAST_LIB=/storage/hive/project/phy-otte/shared/analysistools/SPB2/ROBAST/ROBAST-2.4.4/
export COAST_DIR=/storage/hive/project/phy-otte/shared/analysistools/CORSIKA/corsika/coast-v4r5

export ROOTSYS=/usr/local/pace-apps/manual/packages/root/6.08.06/gcc-8.3.0
export PATH=$PATH:$ROOTSYS/bin
export LD_LIBRARY_PATH=$ROOTSYS/lib/root:${ROBAST_LIB}:${COAST_DIR}/lib:/storage/hive/project/phy-otte/shared/analysistools/VERITAS/lib:${LD_LIBRARY_PATH}
export SLURM_NPROCS=200

cd $SLURM_SUBMIT_DIR
ARGS="/storage/hive/project/phy-otte/omatamala3/Scripts/RunPyLauncher.py ${INFILE}"

#conda activate chasm

while IFS= read -r line; do
    srun --export=ALL --exclusive --ntasks=1 ${line} &

done < "${INFILE}"

wait



srun --export=ALL python ${ARGS}
