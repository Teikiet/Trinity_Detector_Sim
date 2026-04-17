#!/bin/bash

EXECUTESCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/executeCIOReaderParse.sh
PARSESCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/ParseCIOReaderOutput.sh
INPUTDIR=$1


for FLDR in ${INPUTDIR}/Tilt*/; do
	ARGUMENTS="-e ${FLDR}/err-p -o ${FLDR}/out-p -v ${EXECUTESCRIPT} ${PARSESCRIPT} ${FLDR}"
	sbatch ${ARGUMENTS}	
done
