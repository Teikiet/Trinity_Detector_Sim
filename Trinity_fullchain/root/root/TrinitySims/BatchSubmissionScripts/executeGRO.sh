
#!/bin/bash
INFILE=$1
cd /root/TrinitySims/GrOptics/
# Run each command in the input file sequentially
while IFS= read -r line; do
    ${line}
done < "$INFILE"
