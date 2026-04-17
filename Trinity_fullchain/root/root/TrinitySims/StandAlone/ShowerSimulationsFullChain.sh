#!/bin/bash

SIMDIR=/storage/hive/project/phy-otte/omatamala3/Sims
CHASMDIR=${SIMDIR}/CHASM_NuSpacesim
CIODIR=${SIMDIR}/corsikaIOreader
GROPDIR=${SIMDIR}/GrOptics
CAREDIR=${SIMDIR}/CARE

CHASM=${CHASMDIR}/demo/RunCHASMSPB2.py
CIO=${CIODIR}/corsikaIOreader
GROP=${GROPDIR}/grOptics
CARE=${CAREDIR}/CameraAndReadout


CFGGRO=${GROPDIR}/Config/opticsSimulation.pilot
CFGCARE=/storage/home/hhive1/omatamala3/CARE/Camera_Cfg/Events/Flight/SPB2_CARE_Camera_F3Filter.cfg

EXECUTESCRIPT=/storage/hive/project/phy-otte/omatamala3/Scripts/StandAlone/executeFullChain.sh




#No modifications beyond this point


DATE=$(date '+%Y%m%d')

FINALDIRBASE=$(realpath $1)
INPUTDIR=$(realpath $2)


MAXTASKS=500

FINALDIRLOG=${FINALDIRBASE}/${DATE}/LOG

FINALDIRCH=${FINALDIRBASE}/${DATE}/CHASM
FINALDIRCIO=${FINALDIRBASE}/${DATE}/CIO
FINALDIRGROP=${FINALDIRBASE}/${DATE}/GROP
FINALDIRCARE=${FINALDIRBASE}/${DATE}/CARE
FINALDIRINS=${FINALDIRBASE}/${DATE}/SimIns

mkdir -p ${FINALDIRCH}
mkdir -p ${FINALDIRCIO}
mkdir -p ${FINALDIRGROP}
mkdir -p ${FINALDIRCARE}
mkdir -p ${FINALDIRINS}
mkdir -p ${FINALDIRLOG}

FCFLAG=true
filecount=0
linecount=0
shwrcount=0

for FLDR in ${INPUTDIR}/Tilt*/; do

        LAST_DIR=$(basename ${FLDR})
        echo ${LAST_DIR}
        FINALDIRSHCH=${FINALDIRCH}/${LAST_DIR}/
        FINALDIRSHCIO=${FINALDIRCIO}/${LAST_DIR}/
        FINALDIRSHGROP=${FINALDIRGROP}/${LAST_DIR}/
        FINALDIRSHCARE=${FINALDIRCARE}/${LAST_DIR}/

        mkdir -p ${FINALDIRSHCH}
        mkdir -p ${FINALDIRSHCIO}
        mkdir -p ${FINALDIRSHGROP}
        mkdir -p ${FINALDIRSHCARE}
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

                                ARGUMENTSGRO="-pos 1"
                                for (( i=11; i<14; i++ )); do
                                        ARGUMENTSGRO="${ARGUMENTSGRO} ${tokens[i]}"
                                done
                                ARGUMENTSGRO="${ARGUMENTSGRO} -rot 1"
                                for (( i=14; i<16; i++ )); do
                                        ARGUMENTSGRO="${ARGUMENTSGRO} ${tokens[i]}"
                                done

#                               echo "${CHASM} ${ARGUMENTS} ${FINALDIRBASE}/CHASM/${BASENAME%.*}.dat"
				OUTFILEBASE=${BASENAME%.*}_$((shwrcount))
                                echo "conda run -n chasm python3.10 ${CHASM} ${ARGUMENTS} ${FINALDIRSHCH}/${OUTFILEBASE}.dat" > ${FINALDIRINS}/Shower_Sim_${shwrcount}.in
				echo "${CIO} -queff 1.0 -cors ${FINALDIRSHCH}/${OUTFILEBASE}.dat -seed $((RANDOM)) -grisu ${FINALDIRSHCIO}/${OUTFILEBASE}.cph -absfile noExtinction" >> ${FINALDIRINS}/Shower_Sim_${shwrcount}.in
                                echo "${GROP} -p ${CFGGRO} -if ${FINALDIRSHCIO}/${OUTFILEBASE}.cph -of ${FINALDIRSHGROP}/${OUTFILEBASE}.root ${ARGUMENTSGRO}" >> ${FINALDIRINS}/Shower_Sim_${shwrcount}.in
				echo "${CARE} --seed $((RANDOM)) --configfile ${CFGCARE} --outputfile ${FINALDIRSHCARE}/${OUTFILEBASE} --inputfile ${FINALDIRSHGROP}/${OUTFILEBASE}.root --vbfrunnumber 1000 --writepedestals 1" >> ${FINALDIRINS}/Shower_Sim_${shwrcount}.in
				
				SBARGS="-e ${FINALDIRLOG}/err-${shwrcount} -o ${FINALDIRLOG}/out-${shwrcount} -v ${EXECUTESCRIPT} ${FINALDIRINS}/Shower_Sim_${shwrcount}.in"
		                
				RUNNINGJOBS=$(squeue -u omatamala3 | grep omatam | wc -l)
				
				if [ ${RUNNINGJOBS} -lt ${MAXTASKS} ]; then

					sbatch --wait-all-nodes 1 ${SBARGS}
				else
					while [ ${RUNNINGJOBS} -ge ${MAXTASKS} ]; do
						sleep 600
						RUNNINGJOBS=$(squeue -u omatamala3 | grep omatam | wc -l)
					done
					sbatch --wait-all-nodes 1 ${SBARGS}
					
				fi							
                                ((shwrcount++))
                        done < ${FL}

                        ((filecount++))
                        echo "File: ${filecount}"
                done
        fi

done



#echo "${FINALDIR}/CHASM_Sim.in" > ${FINALDIR}/${INPUTMASK}

#ARGUMENTS="-e ${FINALDIR}/err -o ${FINALDIR}/out -v ${EXECUTESCRIPT} ${FINALDIR}/${INPUTMASK}"
#echo ${ARGUMENTS}

#BATCHDIR=${FINALDIR}/SubmissionBatches
#BATCHJOBS=ChasmBatch
#mkdir -p ${BATCHDIR}
#mkdir -p ${FINALDIR}/Errors
#mkdir -p ${FINALDIR}/Output

#linecount=0
#batchcount=0
#while IFS= read -r line; do
#        if (( linecount == 0 )); then
#                echo "$line" > ${BATCHDIR}/${BATCHJOBS}_${batchcount}.in
#        else
#                echo "$line" >> ${BATCHDIR}/${BATCHJOBS}_${batchcount}.in
#        fi
#        ((linecount++))
#        if (( linecount == MAXTASKS )); then
#                linecount=0
#                ARGUMENTS="-e ${FINALDIR}/Errors/err-${batchcount} -o ${FINALDIR}/Output/out-${batchcount} -v ${EXECUTESCRIPT} ${BATCHDIR}/${BATCHJOBS}_${batchcount}.in"
#               echo "${ARGUMENTS}"
#                ((batchcount++))

#        fi

#done <  "${FINALDIR}/CHASM_Sim.in
