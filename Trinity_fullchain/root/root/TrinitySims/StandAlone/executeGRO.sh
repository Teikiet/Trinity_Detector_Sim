#!/bin/bash
#SBATCH -A hive-aotte6
#SBATCH -J GROP
#-n 2
# --cpus-per-task=1
#SBATCH --mem-per-cpu=3G
#SBATCH --time=8:00:00
#SBATCH -p hive

INFILE=$1

#module load anaconda3
#module load pylauncher/4.0
#module load gcc/8.3.0-rv4av7
#module load gsl
#module load root

#ROBAST_LIB=/storage/hive/project/phy-otte/shared/analysistools/SPB2/ROBAST/ROBAST-2.4.4/
#export COAST_DIR=/storage/hive/project/phy-otte/shared/analysistools/CORSIKA/corsika/coast-v4r5

#export ROOTSYS=/usr/local/pace-apps/manual/packages/root/6.08.06/gcc-8.3.0
#export PATH=$PATH:$ROOTSYS/bin
#export LD_LIBRARY_PATH=$ROOTSYS/lib/root:${ROBAST_LIB}:${COAST_DIR}/lib:/storage/hive/project/phy-otte/shared/analysistools/VERITAS/lib:${LD_LIBRARY_PATH}


cd /storage/hive/project/phy-otte/shared/analysistools/TrinitySims/GrOptics
#ARGS="/storage/hive/project/phy-otte/omatamala3/Scripts/RunPyLauncher.py ${INFILE}"

#srun --export=ALL python ${ARGS}
while IFS= read -r line; do

    #srun --exact --mem=0 --ntasks=1 --export=ALL --wait=0 --kill-on-bad-exit=0 --exclusive -c 1 -N1 ${line} &
    ${line}

done < "${INFILE}"
wait
