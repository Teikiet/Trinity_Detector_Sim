# script to execute CARE from an input file and produce the list of output files

# input file: .in file with each line being the run care command. Just execute each line in sequence
# output file: none, output files are specified by the command args


INPUTFILE=$1

cd /root/TrinitySims/CARE/


while IFS= read -r line; do
    ${line}
done < "$INPUTFILE"

wait
echo "All CARE jobs completed."