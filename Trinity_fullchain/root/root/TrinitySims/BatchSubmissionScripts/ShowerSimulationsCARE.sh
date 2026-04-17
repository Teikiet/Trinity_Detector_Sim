#!/bin/bash


# This script generates batch submission files for CARE simulatiosn (electronics) based on GrOptics files. 
# CIODIR=/root/TrinitySims/corsikaIOreader/
# CIO=/root/TrinitySims/corsikaIOreader/corsikaIOreader
# CIOCFG=/root/TrinitySims/corsikaIOreader/data/TLLOC.dat
# EXECUTESCRIPT=/root/TrinitySims/BatchSubmissionScripts/executeCIO.sh

CAREDIR=/root/TrinitySims/CARE/
CAREEXEC=/root/TrinitySims/CARE/CameraAndReadout
CARECFG=/root/TrinitySims/CARE/config/Trinity_Demonstrator_CARE.cfg
EXECUTESCRIPT=/root/TrinitySims/BatchSubmissionScripts/executeCARE.sh



# USAGE
# $1: base output directory
# $2: input directory containing GrOptics files
# $3: optional, seed for CARE runs
# $4: date string (e.g., 20240515)
# $5: number of processors required

DATE=$(date '+%Y%m%d')

MAXTASKS=500
MAXJOBS=400



FINALDIRBASE=$(realpath "$1") # base output directory
INPUTDIR=$(realpath "$2") # input directory containing GrOptics files
# optional, seed for CARE runs
USESEED=$3
DATE=$4
PROCSREQ=$5

# set RANDFLAG to true by default
RANDFLAG=true

# if USESEED is not zero, set RANDFLAG to false, set SEED variable to USESEED
if [ "$USESEED" != "0" ]; then
	echo "Using provided CARE seed: $USESEED"
	SEED=$USESEED
	RANDFLAG=false
else 
	echo "Will use random seeds for each CARE run."
fi

FINALDIR=${FINALDIRBASE}/CARE
#FINALDIRGRO=${FINALDIRBASE}/${DATE}/GROPT
mkdir -p ${FINALDIR}


FCFLAG=true
filecount=0
linecount=0
shwrcount=0
for FLDR in ${INPUTDIR}/Tilt*/; do

	LAST_DIR=$(basename ${FLDR})
	echo ${LAST_DIR}
	FINALDIRSH=${FINALDIR}/${LAST_DIR}/
	mkdir -p ${FINALDIRSH}
	if [ -d ${FLDR} ]; then
		for FL in ${FLDR}/*root; do

			# check if RANDFLAG is true or false, true means no seed provided and we use random seed
			if [ "$RANDFLAG" = true ]; then
				SEED=$RANDOM
			fi
			echo "Processing file: ${FL}"
			echo "Seed used: ${SEED}"

			BASENAME=$(basename "${FL}")

#			echo ${FL}
			linecount=0

			if [ ${FCFLAG} = true ]; then
				echo "${CAREEXEC} -c ${CARECFG} -s ${SEED} -if ${FL} -of ${FINALDIRSH}/${BASENAME%.*} --seed ${SEED}" >> ${FINALDIR}/CARE_Sim.in
				FCFLAG=false
			else
				echo "${CAREEXEC} -c ${CARECFG} -s ${SEED} -if ${FL} -of ${FINALDIRSH}/${BASENAME%.*} --seed ${SEED}" >> ${FINALDIR}/CARE_Sim.in
			fi
			((shwrcount++))
		done
		
		((filecount++))
		echo "File: ${filecount}"
	
	fi

#	echo "-A hive-aotte6 -p${QUEUE} -e ${FINALDIR}/err -o ${FINALDIR}/out -v arg1=${FINALDIR}/CHASM_Sim.in ${EXECUTESCRIPT}"
	
done

echo "${FINALDIR}/CARE_Sim.in" > ${FINALDIR}/Input.txt