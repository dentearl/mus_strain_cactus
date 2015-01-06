#!/bin/bash
# Run this from experiments/coverage_plotting
# must be run with /hive/groups/recon/local/bin/python as your python,
# or the plots will look terrible!
set -o errexit
set -o pipefail
set -o nounset

# Input location constants
# Location of gencode LNCRNA gtf or gff3
LNCRNA_GTF=/hive/users/jcarmstr/msca/gencode/gencode.v2.long_noncoding_RNAs.gtf
# Location of "basestats" files created by mrnaBasesMapped
# Should be changed depending on the release.
BASESTATS_DIR=/hive/users/markd/gencode/mus_strain_cactus/cactusMapCheck/experiments/1409/results/chained/

LNCRNA_IDS=$(mktemp)
TMPDIR=$(mktemp -d)
grep -E -o 'ENSMUST[0-9.]+' ${LNCRNA_GTF} | sort | uniq > ${LNCRNA_IDS}
for i in ${BASESTATS_DIR}/*.basestats; do
    genome=$(basename $i)
    genome=${genome%%.*}
    # get the lncRNA
    grep -F -f ${LNCRNA_IDS} "${i}" > ${TMPDIR}/${genome}.lncRNA
    # get the CDS
    grep -v -F -f ${LNCRNA_IDS} "${i}" > ${TMPDIR}/${genome}.CDS
done

./coverage_plotter.py ${TMPDIR}/*.lncRNA --out lncRNA_coverage --flip
./coverage_plotter.py ${TMPDIR}/*.CDS --out CDS_coverage --flip
