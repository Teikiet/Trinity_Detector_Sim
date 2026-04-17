#!/bin/bash

GROPDIR=/root/TrinitySims/GrOptics/
GROP=${GROPDIR}/grOptics
EXECUTESCRIPT=/root/TrinitySims/BatchSubmissionScripts/executeGRO.sh
ORIGINALCFG=${GROPDIR}/Config/TDEMSimulation.pilot
DATE=$(date '+%Y%m%d')

if [ $# -lt 3 ]; then
  echo "Usage: $0 <FinalDirBase> <InputDir> <GrOpticsParamsFile>"
  echo -e "<FinalDirBase>\t\tThe basename of the directory where the output will be saved. Output will be saved in <FinalDirBase>/DATE/GROPT/Tilt_Angle"
  echo -e "<InputDir>\t\tDirectory where the ShowerCollection.cph file is located"
  echo -e "<GrOpticsParamsFile>\tFile with position and rotation parameters for Groptics"
  exit 1
fi


FINALDIRBASE=$(realpath $1)
INPUTDIR=$(realpath $2)
# GROPPARS=$(realpath $3)
USESEED=$3
DATE=$4
PROCSREQ=$5


FINALDIR=${FINALDIRBASE}/${DATE}/GROPT
mkdir -p ${FINALDIR}


FCFLAG=true
filecount=0
linecount=0
shwrcount=0
USESEED=${USESEED:-0}  # Default to 0 if not provided

# if USESEED is provided, set RANDLFAG to false, set SEED variable to USESEED
if [ "$USESEED" != "0" ]; then
	echo "Using provided GROPT seed: $USESEED"
else 
	echo "No seed provided, will use random seeds for each GROPT run."
fi
# modify the CFG file to set the SEED value
# copy the CFG to FINALDIR to avoid overwriting original
cp ${ORIGINALCFG} ${FINALDIR}/TDEMSimulation.pilot
CFG=${FINALDIR}/TDEMSimulation.pilot


sed -i "s/^\* SEED .*/\* SEED ${USESEED}/" ${CFG}

lineID=0
for FLDR in ${INPUTDIR}/Tilt*/; do

	LAST_DIR=$(basename ${FLDR})
	echo ${LAST_DIR}
	FINALDIRSH=${FINALDIR}/${LAST_DIR}/
	mkdir -p ${FINALDIRSH}


	if [ -d ${FLDR} ]; then

		((linecount++))
#		ARGUMENTS=$(sed -n "${linecount}p" ${GROPPARS})

		for FL in ${FLDR}/*.cph; do
			if [ -f ${FL} ]; then
				CPHFILE=${FL}
				CPHFILEBASE=$(basename ${CPHFILE})
				lineID=${CPHFILEBASE##*_}
				lineID=${lineID%.*}
				((lineID++))
				#ARGUMENTS=$(sed -n "${lineID}p" ${GROPPARS})
				#ARGUMENTS=-rot${ARGUMENTS#*-rot}
				if [ ${FCFLAG} = true ]; then
					#echo "${GROP} -p ${CFG} -if ${CPHFILE} -of ${FINALDIRSH}/${CPHFILEBASE%.*}.root ${ARGUMENTS}" > ${FINALDIR}/GROP_Sim.in
					echo "${GROP} -p ${CFG} -if ${CPHFILE} -of ${FINALDIRSH}/${CPHFILEBASE%.*}.root" > ${FINALDIR}/GROP_Sim.in

					FCFLAG=false
				else
					#echo "${GROP} -p ${CFG} -if ${CPHFILE} -of ${FINALDIRSH}/${CPHFILEBASE%.*}.root ${ARGUMENTS}" >> ${FINALDIR}/GROP_Sim.in
					echo "${GROP} -p ${CFG} -if ${CPHFILE} -of ${FINALDIRSH}/${CPHFILEBASE%.*}.root" >> ${FINALDIR}/GROP_Sim.in			

				fi
			fi
		done
	fi
	
done

totalLines=$(wc -l < "${FINALDIR}/GROP_Sim.in")
parsedLines=0
echo "GrOptics input file prepared: ${FINALDIR}/GROP_Sim.in"
