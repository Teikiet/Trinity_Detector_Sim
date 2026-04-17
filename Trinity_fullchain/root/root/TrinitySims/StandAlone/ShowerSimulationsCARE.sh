#!/bin/bash

CAREDIR=/storage/hive/project/phy-otte/jbogdan3/Sims/CARE/
CARE=${CAREDIR}/CameraAndReadout
EXECUTESCRIPT=/storage/hive/project/phy-otte/jbogdan3/Sims/StandAlone/executeCARE.sh
CFG=/storage/hive/project/phy-otte/jbogdan3/Sims/CARE/config/SPB2_CARE_Camera_F3Filter.cfg
DATE=$(date '+%Y%m%d')

if [ $# -lt 2 ]; then
  echo "Usage: $0 <FinalDirBase> <InputDir>"
  echo -e "<FinalDirBase>\t\tThe basename of the directory where the output will be saved. Output will be saved in <FinalDirBase>/DATE/GROPT/Tilt_Angle"
  echo -e "<InputDir>\t\tDirectory where the ShowerCollection_XX.root file is located"
  exit 1
fi


FINALDIRBASE=$(realpath $1)
INPUTDIR=$(realpath $2)
DATE=$3
PROCSREQ=$4


FINALDIR=${FINALDIRBASE}/${DATE}/CARE
mkdir -p ${FINALDIR}


FCFLAG=true
filecount=0
linecount=0
shwrcount=0

SRUN=${RANDOM}
VBFRUNNUMBER=10000
WRITEPEDESTAL=1
for FLDR in ${INPUTDIR}/Tilt*/; do

	LAST_DIR=$(basename ${FLDR})
	echo ${LAST_DIR}
	FINALDIRSH=${FINALDIR}/${LAST_DIR}/
	mkdir -p ${FINALDIRSH}


	if [ -d ${FLDR} ]; then

		((linecount++))

		for FL in ${FLDR}/*.root; do
			if [ -f ${FL} ]; then
				CPHFILE=${FL}
				CPHFILEBASE=$(basename ${CPHFILE})
				
				if [ ${FCFLAG} = true ]; then
					echo "${CARE} --seed $((SRUN)) --configfile ${CFG} --outputfile ${FINALDIRSH}/${CPHFILEBASE%.*} --inputfile ${CPHFILE} --vbfrunnumber ${VBFRUNNUMBER} --writepedestals ${WRITEPEDESTAL}" > ${FINALDIR}/CARE_Sim.in

					FCFLAG=false
				else
					echo "${CARE} --seed $((SRUN)) --configfile ${CFG} --outputfile ${FINALDIRSH}/${CPHFILEBASE%.*} --inputfile ${CPHFILE} --vbfrunnumber ${VBFRUNNUMBER} --writepedestals ${WRITEPEDESTAL}" >> ${FINALDIR}/CARE_Sim.in


				fi
			fi
		done
	fi
	
done

echo "${FINALDIR}/CARE_Sim.in" > ${FINALDIR}/Input.txt

#ARGUMENTS="-e ${FINALDIR}/err -o ${FINALDIR}/out -v ${EXECUTESCRIPT} ${FINALDIR}/Input.txt"
#echo ${ARGUMENTS}
#sbatch ${ARGUMENTS}

MAXTASKS=500
MAXJOBS=400

BATCHDIR=${FINALDIR}/SubmissionBatches
BATCHJOBS=CAREBatch
mkdir -p ${BATCHDIR}
mkdir -p ${FINALDIR}/Errors
mkdir -p ${FINALDIR}/Output

linecount=0
batchcount=0
bundlecount=0
batchcount=0
totalLines=$(wc -l < "${FINALDIR}/CARE_Sim.in")
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
					procscount=0
					ARGUMENTS="-e ${FINALDIR}/Errors/err-${bundlecount} -o ${FINALDIR}/Output/out-${bundlecount} -v ${EXECUTESCRIPT} ${BATCHDIR}/${BATCHJOBS}_${bundlecount}.in"
					RUNNINGJOBS=$(squeue -u jbogdan3 | grep jbog | wc -l)

					if [ ${RUNNINGJOBS} -lt ${MAXJOBS} ]; then

						jobID=$(sbatch --wait-all-nodes 1 -n ${PROCSREQ} --cpus-per-task=1 ${ARGUMENTS} | awk '{print $NF}')
					else
						while [ ${RUNNINGJOBS} -ge ${MAXJOBS} ]; do
							sleep 60
							RUNNINGJOBS=$(squeue -u jbogdan3 | grep jbog | wc -l)
						done
						jobID=$(sbatch --wait-all-nodes 1 -n ${PROCSREQ} --cpus-per-task=1 ${ARGUMENTS} | awk '{print $NF}')

					fi
					((bundlecount++))
				fi
				((batchcount++))

        fi

done <  "${FINALDIR}/CARE_Sim.in"
echo $jobID
