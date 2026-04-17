#!/bin/bash

CIODIR=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/corsikaIOreader
CIO=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/corsikaIOreader/corsikaIOreader
CIOCFG=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/corsikaIOreader/data/TLLOC.dat
EXECUTESCRIPT=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/StandAlone/executeCIO.sh
DATE=$(date '+%Y%m%d')

MAXTASKS=500
MAXJOBS=400



FINALDIRBASE=$(realpath "$1")
INPUTDIR=$(realpath "$2")
DATE=$3
PROCSREQ=$4

FINALDIR=${FINALDIRBASE}/${DATE}/CIO
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
		for FL in ${FLDR}/*dat; do
			BASENAME=$(basename "${FL}")

#			echo ${FL}
			linecount=0

			if [ ${FCFLAG} = true ]; then
				echo "${CIO} -queff 1.0 -cors ${FL} -seed $((RANDOM)) -grisu ${FINALDIRSH}/${BASENAME%.*}.cph -absfile noExtinction" > ${FINALDIR}/CIO_Sim.in
				FCFLAG=false
			else
				echo "${CIO} -queff 1.0 -cors ${FL} -seed $((RANDOM)) -grisu ${FINALDIRSH}/${BASENAME%.*}.cph -absfile noExtinction" >> ${FINALDIR}/CIO_Sim.in
			fi
			((shwrcount++))
		done
		
		((filecount++))
		echo "File: ${filecount}"
	
	fi

#	echo "-A hive-aotte6 -p${QUEUE} -e ${FINALDIR}/err -o ${FINALDIR}/out -v arg1=${FINALDIR}/CHASM_Sim.in ${EXECUTESCRIPT}"
	
done

echo "${FINALDIR}/CIO_Sim.in" > ${FINALDIR}/Input.txt

#ARGUMENTS="-e ${FINALDIR}/err -o ${FINALDIR}/out -v ${EXECUTESCRIPT} ${FINALDIR}/Input.txt"
#echo ${ARGUMENTS}
#sbatch ${ARGUMENTS}


BATCHDIR=${FINALDIR}/SubmissionBatches
BATCHJOBS=CIOBatch
mkdir -p ${BATCHDIR}
mkdir -p ${FINALDIR}/Errors
mkdir -p ${FINALDIR}/Output

linecount=0
batchcount=0
bundlecount=0
procscount=0

PARTITIONS=("hive")
pidx=0

totalLines=$(wc -l < "${FINALDIR}/CIO_Sim.in")
parsedLines=0
#while IFS= read -r line; do
while false; do
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
						jobID=$(sbatch -p ${PARTITIONS[$pidx]} --wait-all-nodes 1 -n ${PROCSREQ} --cpus-per-task=1 ${ARGUMENTS} | awk '{print $NF}')
                	else
                        while [ ${RUNNINGJOBS} -ge ${MAXJOBS} ]; do
                             sleep 120
                             RUNNINGJOBS=$(squeue -u jbogdan3 | grep jbog | wc -l)
                        done
                        jobID=$(sbatch -p ${PARTITIONS[$pidx]} --wait-all-nodes 1  -n ${PROCSREQ} --cpus-per-task=1 ${ARGUMENTS} | awk '{print $NF}')

                 	fi
					((bundlecount++))
				fi

                ((batchcount++))

        fi

done <  "${FINALDIR}/CIO_Sim.in"
echo $jobID
