#!/bin/bash
#SBATCH -A hive-aotte6
#SBATCH -J IFHB
#SBATCH -n 2
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=1G
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=jbogdan3@gatech.edu
#SBATCH -p hive

INFILE=$1

#module load anaconda3
#module load pylauncher
#module load gcc/8.3.0-rv4av7
#module load gsl
module load root

#ROBAST_LIB=/storage/hive/project/phy-otte/shared/analysistools/SPB2/ROBAST/ROBAST-2.4.4/
#export COAST_DIR=/storage/hive/project/phy-otte/shared/analysistools/CORSIKA/corsika/coast-v4r5

#export ROOTSYS=/usr/local/pace-apps/manual/packages/root/6.08.06/gcc-8.3.0
#export PATH=$PATH:$ROOTSYS/bin
#export LD_LIBRARY_PATH=$ROOTSYS/lib/root:${ROBAST_LIB}:${COAST_DIR}/lib:/storage/hive/project/phy-otte/shared/analysistools/VERITAS/lib:${LD_LIBRARY_PATH}

num_lines=$(wc -l < "${INFILE}")

MAXNJOBS=200

submitted_jobs=0

# Loop over each line in the input file and submit srun commands in parallel
while IFS= read -r line; do
    srun --export=ALL --exclusive --ntasks=1 ${line} &
    #${line}

done < "${INFILE}"

wait
