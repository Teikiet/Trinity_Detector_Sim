#!/bin/bash

GROPDIR=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/GrOptics
GROP=${GROPDIR}/grOptics
EXECUTESCRIPT=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/StandAlone/executeGRO.sh
CFG=${GROPDIR}/Config/TDEMSimulation.pilot
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
GROPPARS=$(realpath $3)
DATE=$4
PROCSREQ=$5


FINALDIR=${FINALDIRBASE}/${DATE}/GROPT
mkdir -p ${FINALDIR}


FCFLAG=true
filecount=0
linecount=0
shwrcount=0

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

echo "${FINALDIR}/GROP_Sim.in" > ${FINALDIR}/Input.txt

#ARGUMENTS="-e ${FINALDIR}/err -o ${FINALDIR}/out -v ${EXECUTESCRIPT} ${FINALDIR}/Input.txt"
#echo ${ARGUMENTS}
#sbatch ${ARGUMENTS}

MAXTASKS=500
MAXJOBS=400

BATCHDIR=${FINALDIR}/SubmissionBatches
BATCHJOBS=GrOBatch
mkdir -p ${BATCHDIR}
mkdir -p ${FINALDIR}/Errors
mkdir -p ${FINALDIR}/Output

linecount=0
batchcount=0
bundlecount=0
procscount=0
totalLines=$(wc -l < "${FINALDIR}/GROP_Sim.in")
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
					procscount=0
				    ARGUMENTS="-e ${FINALDIR}/Errors/err-${bundlecount} -o ${FINALDIR}/Output/out-${bundlecount} -v ${EXECUTESCRIPT} ${BATCHDIR}/${BATCHJOBS}_${bundlecount}.in"
               		RUNNINGJOBS=$(squeue -u jbogdan3 | grep jbog | wc -l)

                	if [ ${RUNNINGJOBS} -lt ${MAXJOBS} ]; then

                 		
						jobID=$(sbatch --wait-all-nodes 1 -n ${PROCSREQ} --cpus-per-task=1 ${ARGUMENTS}  | awk '{print $NF}')
                 	else
                        while [ ${RUNNINGJOBS} -ge ${MAXJOBS} ]; do
                             sleep 120
                             RUNNINGJOBS=$(squeue -u jbogdan3 | grep jbog | wc -l)
                        done
                        jobID=$(sbatch --wait-all-nodes 1 -n ${PROCSREQ} --cpus-per-task=1 ${ARGUMENTS}  | awk '{print $NF}')
	                fi
				 ((bundlecount++))
				fi
            ((batchcount++))

        fi

done <  "${FINALDIR}/GROP_Sim.in"
echo $jobID
