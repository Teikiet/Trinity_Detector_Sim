#!/bin/bash

CIODIR=/root/TrinitySims/corsikaIOreader/
CIO=/root/TrinitySims/corsikaIOreader/corsikaIOreader
CIOCFG=/root/TrinitySims/corsikaIOreader/data/TLLOC.dat
EXECUTESCRIPT=/root/TrinitySims/BatchSubmissionScripts/executeCIO.sh
DATE=$(date '+%Y%m%d')

MAXTASKS=500
MAXJOBS=400



FINALDIRBASE=$(realpath "$1")
INPUTDIR=$(realpath "$2")
USESEED=$3 # seed for CIO runs, 0 means random seeds
DATE=$4
PROCSREQ=$5

FINALDIR=${FINALDIRBASE}/${DATE}/CIO
#FINALDIRGRO=${FINALDIRBASE}/${DATE}/GROPT
mkdir -p ${FINALDIR}


FCFLAG=true
filecount=0
linecount=0
shwrcount=0
RANDFLAG=true
USESEED=${USESEED:-0}  # Default to 0 if not provided

# if USESEED is not zero, set RANDFLAG to false, set SEED variable to USESEED
if [ "$USESEED" != "0" ]; then
	echo "Using provided CIO seed: $USESEED"
	SEED=$USESEED
	RANDFLAG=false
else 
	echo "Will use random seeds for each CIO run."
fi
# echo "SEED: ${SEED} RANDFLAG: ${RANDFLAG}"


for FLDR in ${INPUTDIR}/Tilt*/; do

	LAST_DIR=$(basename ${FLDR})
	echo ${LAST_DIR}
	FINALDIRSH=${FINALDIR}/${LAST_DIR}/
	mkdir -p ${FINALDIRSH}
	if [ -d ${FLDR} ]; then
		for FL in ${FLDR}/*dat; do
			BASENAME=$(basename "${FL}")
			
			# check if RANDFLAG is true or false, true means no seed provided and we use random seed
			if [ "$RANDFLAG" = true ]; then
				SEED=$RANDOM
			fi
#			echo ${FL}
			linecount=0

			if [ ${FCFLAG} = true ]; then
				# seed is set above, either from input or random
				echo "${CIO} -queff 1.0 -cors ${FL} -seed ${SEED} -grisu ${FINALDIRSH}/${BASENAME%.*}.cph -absfile noExtinction" > ${FINALDIR}/CIO_Sim.in
				FCFLAG=false
			else
				echo "${CIO} -queff 1.0 -cors ${FL} -seed ${SEED} -grisu ${FINALDIRSH}/${BASENAME%.*}.cph -absfile noExtinction" >> ${FINALDIR}/CIO_Sim.in
			fi
			((shwrcount++))
		done
		
		((filecount++))
		echo "File: ${filecount}"
	
	fi

#	echo "-A hive-aotte6 -p${QUEUE} -e ${FINALDIR}/err -o ${FINALDIR}/out -v arg1=${FINALDIR}/CHASM_Sim.in ${EXECUTESCRIPT}"
	
done

echo "CIOReader input file prepared: ${FINALDIR}/CIO_Sim.in"
