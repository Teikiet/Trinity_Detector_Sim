#!/bin/bash

DATE=$(date '+%Y%m%d')
ISOG=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/ifhb/IStayOnGround
CONFIGFILE=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/ifhb/ISOG.pilot
EXECUTESCRIPT=/storage/hive/project/phy-otte/shared/analysistools/TrinitySims/StandAlone/executeISOG.sh

FINALDIRBASE=$1
BASENAME=$2
DATE=$3

TLOW=91.56
TUP=91.56
TSTEP=1

ZLOW=0
ZUP=0
ZSTEP=1.0

AZLOW=0
AZUP=0
AZSTEP=1

ELOW=8
EUP=8
ESTEP=1.0


numBSM=1
baseBSMFactor=$(echo "(2.0/$numBSM)" | bc -l )
startBSM=1
numBSM=0
ENU=${ELOW}
ZA=${ZLOW}
AZ=${AZLOW}
TLT=${TLOW}

FINALDIR=${FINALDIRBASE}/${DATE}/ISOG
mkdir -p ${FINALDIR}

POINTSOURCE=0
FCFLAG=true
while (( $(echo "${TLT} <= ${TUP}" | bc -l) )); do
	ZA=${ZLOW}
	FMTTLT=$(printf "%03f" "$TLT")
	mkdir -p ${FINALDIR}/Tilt_${FMTTLT}
	while (( $(echo "${ZA} <= ${ZUP}" | bc -l) )); do
		AZ=${AZLOW}
		while (( $(echo "${AZ} <= ${AZUP}" | bc -l) )); do
			ENU=${ELOW}
			while (( $(echo "${ENU} <= ${EUP}" | bc -l) )); do
				for ((i = 0; i <= numBSM; i++)); do
					exptt=$(echo "$baseBSMFactor*$i"| bc)
					bsmFactor=$(echo "e($exptt*l(10.0))" | bc -l)
					SRUN=$((RANDOM))
					FMTBSM=$(printf "%.3f" "$bsmFactor")
					ARGUMENTS="${ISOG} -c ${CONFIGFILE} -s $((SRUN)) -of ${FINALDIR}/Tilt_${FMTTLT}/${BASENAME}_BSM_$FMTBSM NEUTRINOENERGY ${ENU} ENERGYSPECTRUM 0 BSMMEANLIFE ${bsmFactor} TELPOINTING ${TLT} ISPOINTSOURCE ${POINTSOURCE} "
					#echo ${ARGUMENTS}
					if [ "$FCFLAG" = true ]; then
						echo ${ARGUMENTS} > ${FINALDIR}/ISOG_Sim.in
						FCFLAG=false
					else
						echo ${ARGUMENTS} >> ${FINALDIR}/ISOG_Sim.in
					fi
				done
				#echo "Energy ${ENU} done"
				ENU=$(echo "${ENU} + ${ESTEP}" | bc)
			done		
			AZ=$(echo "${AZ} + ${AZSTEP}" | bc)
		done
		ZA=$(echo "${ZA} + ${ZSTEP}" | bc)
	done
	TLT=$(echo "${TLT} +${TSTEP}" | bc)	
done

cp ${CONFIGFILE} ${FINALDIR}
QUEUE=hive-nvme
echo "-A hive-aotte6 -p${QUEUE} -e ${FINALDIR}/err -o ${FINALDIR}/out -v arg1=${FINALDIR}/ISOG_Sim.in ${EXECUTESCRIPT}"
ARGUMENTS="-e ${FINALDIR}/err -o ${FINALDIR}/out -v ${EXECUTESCRIPT} ${FINALDIR}/ISOG_Sim.in"
jobID=$(sbatch ${ARGUMENTS} | awk '{print $NF}')

echo "$jobID"
