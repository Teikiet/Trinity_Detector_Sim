#!/bin/bash
INFILE=$1

# Activate conda environment if needed (uncomment if required)
# eval "$(conda shell.bash hook)"
# conda activate

# Run each command in the input file sequentially
while IFS= read -r line; do
    ${line}
done < "$INFILE"
