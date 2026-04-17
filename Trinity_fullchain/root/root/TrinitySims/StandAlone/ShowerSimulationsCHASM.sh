#!/bin/bash

CHASM=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/CHASM_NuSpacesim/demo/RunCHASM.py
EXECUTESCRIPT=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/StandAlone/executeCHASM.sh
DATE=$(date '+%Y%m%d')
INPUTMASK=Input.txt


FINALDIRBASE=$(realpath $1)
INPUTDIR=$(realpath $2)
DATE=$3
PROCSREQ=$4

MAXTASKS=500
MAXJOBS=400

FINALDIR=${FINALDIRBASE}/${DATE}/CHASM
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
		for FL in ${FLDR}/*txt; do
			BASENAME=$(basename "${FL}")

			echo ${FL}
			linecount=0

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
				
				#ARGUMENTSGRO="-pos 1"
				#for (( i=11; i<14; i++ )); do
                #                        ARGUMENTSGRO="${ARGUMENTSGRO} ${tokens[i]}"
                #                done
				#ARGUMENTSGRO="${ARGUMENTSGRO} -rot 1"
				#for (( i=14; i<16; i++ )); do
                #                        ARGUMENTSGRO="${ARGUMENTSGRO} ${tokens[i]}"
                #                done

#				echo "${CHASM} ${ARGUMENTS} ${FINALDIRBASE}/CHASM/${BASENAME%.*}.dat"
				
				if [ ${FCFLAG} = true ]; then
					echo "conda run -n simenv python3.10 ${CHASM} ${ARGUMENTS} ${FINALDIRSH}/${BASENAME%.*}_$((shwrcount)).dat" > ${FINALDIR}/CHASM_Sim.in
					#echo "python3.10 ${CHASM} ${ARGUMENTS} ${FINALDIRSH}/${BASENAME%.*}_$((shwrcount)).dat" > ${FINALDIR}/CHASM_Sim.in
					#echo "${ARGUMENTSGRO}" > ${FINALDIR}/GrOpticsTokens.txt
					FCFLAG=false
				else
					echo "conda run -n simenv python3.10 ${CHASM} ${ARGUMENTS} ${FINALDIRSH}/${BASENAME%.*}_$((shwrcount)).dat" >> ${FINALDIR}/CHASM_Sim.in
					#echo "python3.10 ${CHASM} ${ARGUMENTS} ${FINALDIRSH}/${BASENAME%.*}_$((shwrcount)).dat" >> ${FINALDIR}/CHASM_Sim.in
					#echo "${ARGUMENTSGRO}" >> ${FINALDIR}/GrOpticsTokens.txt
				fi
				((shwrcount++))
			done < ${FL}
			
			((filecount++))
			echo "File: ${filecount}"
		done
	fi

#	echo "-A hive-aotte6 -p${QUEUE} -e ${FINALDIR}/err -o ${FINALDIR}/out -v arg1=${FINALDIR}/CHASM_Sim.in ${EXECUTESCRIPT}"
done
echo "${FINALDIR}/CHASM_Sim.in" > ${FINALDIR}/${INPUTMASK}

ARGUMENTS="-e ${FINALDIR}/err -o ${FINALDIR}/out -v ${EXECUTESCRIPT} ${FINALDIR}/${INPUTMASK}"
echo ${ARGUMENTS}

BATCHDIR=${FINALDIR}/SubmissionBatches
BATCHJOBS=ChasmBatch


mkdir -p ${BATCHDIR}
mkdir -p ${FINALDIR}/Errors
mkdir -p ${FINALDIR}/Output

linecount=0
batchcount=0
bundlecount=0
procscount=0

PARTITIONS=("hive")
pidx=0

jobID=0
totalLines=$(wc -l < "${FINALDIR}/CHASM_Sim.in")
parsedLines=0
while IFS= read -r line; do
	if (( linecount == 0 )); then
		echo '#!/bin/bash' > ${BATCHDIR}/${BATCHJOBS}_${batchcount}.sh
		echo "$line" >> ${BATCHDIR}/${BATCHJOBS}_${batchcount}.sh
	else
		echo "$line" >> ${BATCHDIR}/${BATCHJOBS}_${batchcount}.sh
	fi			
	((linecount++))
	((parsedLines++))
	if [[ $linecount -eq $MAXTASKS  ||  $parsedLines -eq $totalLines ]]; then
		linecount=0
		chmod +x ${BATCHDIR}/${BATCHJOBS}_${batchcount}.sh

		if (( procscount == 0 )); then
			echo "${BATCHDIR}/${BATCHJOBS}_${batchcount}.sh" > ${BATCHDIR}/${BATCHJOBS}_${bundlecount}.in
		else
			echo "${BATCHDIR}/${BATCHJOBS}_${batchcount}.sh" >> ${BATCHDIR}/${BATCHJOBS}_${bundlecount}.in
		fi
		((procscount++))
		if (( procscount == PROCSREQ )); then
			pidx=$(( (bundlecount) % 1 ))
			procscount=0
			ARGUMENTS="-e ${FINALDIR}/Errors/err-${bundlecount} -o ${FINALDIR}/Output/out-${bundlecount} -v ${EXECUTESCRIPT} ${BATCHDIR}/${BATCHJOBS}_${bundlecount}.in"

			RUNNINGJOBS=$(squeue -u jbogdan3 | grep jbog | wc -l)

                if [ ${RUNNINGJOBS} -lt ${MAXJOBS} ]; then
                			
					jobID=$(sbatch -p ${PARTITIONS[$pidx]} --wait-all-nodes 1 -n ${PROCSREQ} --cpus-per-task=1 ${ARGUMENTS}  | awk '{print $NF}')
                else
                    while [ ${RUNNINGJOBS} -ge ${MAXJOBS} ]; do
                        sleep 120
                        RUNNINGJOBS=$(squeue -u jbogdan3 | grep jbog | wc -l)
                    done
                    jobID=$(sbatch -p ${PARTITIONS[$pidx]} --wait-all-nodes 1 -n ${PROCSREQ} --cpus-per-task=1 ${ARGUMENTS}  | awk '{print $NF}')
                fi
			((bundlecount++))
		fi
		((batchcount++))	
	fi
	
done < 	"${FINALDIR}/CHASM_Sim.in"

echo $jobID
