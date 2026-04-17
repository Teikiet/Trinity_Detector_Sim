#!/bin/bash
IFHBSCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/StandAlone/ShowerSimulationsIFHB.sh
CHASMSCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/StandAlone/ShowerSimulationsCHASM.sh
CIOSCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/StandAlone/ShowerSimulationsCIOReader.sh
GROPSCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/StandAlone/ShowerSimulationsGrOptics.sh
CARESCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/StandAlone/ShowerSimulationsCARE.sh

STAGE=$1
OUTDIR=$2
NAMEPREF=$3
PROCSREQ=$4
DATE=$(date '+%Y%m%d')
#

if [ $# -eq 5 ]; then
 	DATE=$5
fi

echo $DATE
#DATE=$(date '+%Y%m%d')
#if [ $STAGE -eq "ALL" ]; then
	flag_ifhb=1
	flag_chasm=1
	flag_cio=1
	flag_gro=1
	flag_care=1
#fi
echo "Stage IFHB: Starting..."
if(($flag_ifhb)); then
	lastJob=$(${IFHBSCRIPT} ${OUTDIR} ${NAMEPREF} $DATE | tail -n 1)
fi
isRunning=$(squeue -u omatamala3 | grep "IFHB" | wc -l)
while [ $isRunning -gt 0 ]; do
	echo "Waiting for previous stage to finish..."
	sleep 300
	isRunning=$(squeue -u omatamala3 | grep "IFHB" | wc -l)
done

echo "Stage CHASM: Starting..."
if(($flag_chasm)); then
	lastJob=$(${CHASMSCRIPT} ${OUTDIR} ${OUTDIR}/${DATE}/IFHB/ ${DATE} ${PROCSREQ} | tail -n 1)
fi
isRunning=$(squeue -u omatamala3 | grep "CHASM" | wc -l)
while [ $isRunning -gt 0 ]; do
	echo "Waiting for previous stage to finish..."
	sleep 300
	isRunning=$(squeue -u omatamala3 | grep "CHASM" | wc -l)
done

echo "Stage CIOReader: Starting..."
if(($flag_cio)); then
	lastJob=$(${CIOSCRIPT} ${OUTDIR} ${OUTDIR}/${DATE}/CHASM/ ${DATE} ${PROCSREQ} | tail -n 1)
fi
isRunning=$(squeue -u omatamala3 | grep "CIOR" | wc -l)
while [ $isRunning -gt 0 ]; do
	echo "Waiting for previous stage to finish..."
	sleep 300
	isRunning=$(squeue -u omatamala3 | grep "CIOR" | wc -l)
done

echo "Stage GrOptics: Starting..."
if(($flag_gro)); then
	lastJob=$(${GROPSCRIPT} ${OUTDIR} ${OUTDIR}/${DATE}/CIO/ ${OUTDIR}/${DATE}/CHASM/GrOpticsTokens.txt ${DATE} ${PROCSREQ} | tail -n 1)
fi
isRunning=$(squeue -u omatamala3 | grep "GROP" | wc -l)
while [ $isRunning -gt 0 ]; do
	echo "Waiting for previous stage to finish..."
	sleep 300
	isRunning=$(squeue -u omatamala3 | grep "GROP" | wc -l)
done

echo "Stage CARE: Starting..."
if(($flag_care)); then
	lastJob=$(${CARESCRIPT} ${OUTDIR} ${OUTDIR}/${DATE}/GROPT/ ${DATE} ${PROCSREQ})
fi
