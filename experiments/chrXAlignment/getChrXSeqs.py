#!/usr/bin/env python
from sonLib.bioio import popenCatch
import sys
genome = sys.argv[1]

chromCoverage = popenCatch("bedtools genomecov -i %s.bed -g %s.chromSizes" % (genome, genome))
for line in chromCoverage.split("\n"):
    line = line.strip()
    if line == "":
        continue
    fields = line.split("\t")
    name = fields[0]
    onOrOff = fields[1]
    percent = float(fields[4])
    if onOrOff == "0":
        # only interested in the % of coverage of a seq, not anti-coverage
        continue
    if percent > 0.80 and name != "genome":
        print name
