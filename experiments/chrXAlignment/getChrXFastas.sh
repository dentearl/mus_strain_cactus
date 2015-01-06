#!/bin/bash
# Requires bedtools and kent tools
set -o errexit
set -o pipefail
set -o nounset

if [ $# -eq 0 ]; then
    echo "Usage: $0 halFile"
    exit 1
fi

HAL_FILE=$1
halStats --bedSequences C57B6J $HAL_FILE | egrep '^X' > chrX.bed

# assumes that all non-leaves are named 'AncNN'
for genome in $(halStats --genomes $HAL_FILE | awk '{for(i=1;i<=NF;i++) { if (!($i ~ /^Anc/)) print $i} }'); do
    (
        # remove --inMemory if you have memory issues.
        halLiftover --inMemory $HAL_FILE C57B6J chrX.bed $genome $genome.bed
        TMPFILE=$(mktemp)
        bedtools sort -i $genome.bed > $TMPFILE
        mv $TMPFILE $genome.bed
        # get chromSizes for bedtools genomecov to use
        halStats --chromSizes $genome $HAL_FILE > $genome.chromSizes
        hal2fasta $HAL_FILE $genome > $genome.fa
        # get the coverage and use it to extract the correct seq names
        python getChrXSeqs.py $genome > $genome.chrX.seqs
        faSomeRecords $genome.fa $genome.chrX.seqs $genome.chrX.fa
    ) & # the process for each genome is independent so doing them all in parallel helps a lot
done
wait # wait for all genomes to be done
