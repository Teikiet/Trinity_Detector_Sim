#!/bin/bash

CHASM=/storage/hive/project/phy-otte/omatamala3/Sims/CHASM_NuSpacesim/demo/RunCHASMSPB2Draw.py
EXECUTESCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/StandAlone/executeCHASMDraw.sh
DATE=$(date '+%Y%m%d')
INPUTMASK=InputDraw.txt
FINALDIRBASE=$1
INPUTDIR=$2


FINALDIR=${FINALDIRBASE}/${DATE}/CHASM
#FINALDIRGRO=${FINALDIRBASE}/${DATE}/GROPT
mkdir -p ${FINALDIR}


FCFLAG=true
filecount=0
linecount=0
shwrcount=0
for FLDR in ${INPUTDIR}/Tilt*/; do

	LAST_DIR=$(basename ${FLDR})
#	FCFLAG=true
	echo ${LAST_DIR}
	FINALDIRSH=${FINALDIR}/${LAST_DIR}/
#	FINALDIRGRO=${FINALDIRBASE}/${DATE}/GROPT/${LAST_DIR}
	mkdir -p ${FINALDIRSH}
#	mkdir -p ${FINALDIRFGRO}
	if [ -d ${FLDR} ]; then
		for FL in ${FLDR}/*txt; do
			BASENAME=$(basename "${FL}")

			echo ${FL}
			linecount=0
#			FCFLAG=true

			while IFS= read -r line; do
				((linecount++))
				#echo $line	
				if [ ${linecount} -le 2 ]; then
					continue
				fi
				#echo $line
				tokens=($line)
	
				ARGUMENTS=""
				for (( i=0; i<${#tokens[@]} && i<11; i++ )); do
					ARGUMENTS="${ARGUMENTS} ${tokens[i]}"
				done
				
				ARGUMENTSGRO="-pos 1"
				for (( i=11; i<14; i++ )); do
                                        ARGUMENTSGRO="${ARGUMENTSGRO} ${tokens[i]}"
                                done
				ARGUMENTSGRO="${ARGUMENTSGRO} -rot 1"
				for (( i=14; i<16; i++ )); do
                                        ARGUMENTSGRO="${ARGUMENTSGRO} ${tokens[i]}"
                                done

#				echo "${CHASM} ${ARGUMENTS} ${FINALDIRBASE}/CHASM/${BASENAME%.*}.dat"
				
				if [ ${FCFLAG} = true ]; then
					echo "conda run -n chasm python3.10 ${CHASM} ${ARGUMENTS} ${FINALDIRSH}/${BASENAME%.*}_$((shwrcount)).dat" > ${FINALDIR}/CHASM_SimDraw.in
					echo "${ARGUMENTSGRO}" > ${FINALDIR}/GrOpticsTokens.txt
					FCFLAG=false
				else
					echo "conda run -n chasm python3.10 ${CHASM} ${ARGUMENTS} ${FINALDIRSH}/${BASENAME%.*}_$((shwrcount)).dat" >> ${FINALDIR}/CHASM_SimDraw.in
					echo "${ARGUMENTSGRO}" >> ${FINALDIR}/GrOpticsTokens.txt
				fi
				((shwrcount++))
			done < ${FL}
			
			((filecount++))
			echo "File: ${filecount}"
		done
	fi

#	echo "-A hive-aotte6 -p${QUEUE} -e ${FINALDIR}/err -o ${FINALDIR}/out -v arg1=${FINALDIR}/CHASM_Sim.in ${EXECUTESCRIPT}"

	
	
done
echo "${FINALDIR}/CHASM_SimDraw.in" > ${FINALDIR}/${INPUTMASK}

ARGUMENTS="-e ${FINALDIR}/errD -o ${FINALDIR}/outD -v ${EXECUTESCRIPT} ${FINALDIR}/${INPUTMASK}"
echo ${ARGUMENTS}
sbatch ${ARGUMENTS}
