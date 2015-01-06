
import os
import sys
import re

#chrom	strand	txStart	txEnd	cdsStart	cdsEnd	exonCount	exonStarts	exonEnds	proteinID	alignID
def write_bed(gpfile, bedfile):
    gpf = open(gpfile, 'r')
    bf = open(bedfile, 'w')
    for line in gpf:
        if re.search('chrom', line):
            continue
        items = line.strip("\n").split("\t")
        starts = [int(s) for s in items[7].rstrip(',').split(',')]
        ends = [int(e) for e in items[8].rstrip(',').split(',')]
        #convert starts & ends to be relative to genestart
        txstart = int(items[2])
        starts = [s - txstart for s in starts]
        ends = [e - txstart for e in ends]

        sizes = [e - starts[i] for i, e in enumerate(ends)]
        for i, start in enumerate(starts):
            assert start >= 0
            assert sizes[i] > 0

        chr = items[0].lstrip("chr")
        chritems = chr.split("_")
        if len(chritems) > 1:
            chr = chritems[1]
        if chr == 'M':
            continue
        starts_str = ",".join([str(start) for start in starts])
        sizes_str = ",".join([str(size) for size in sizes])
        protname = items[9]
        if not protname:
            protname = "na"
        bf.write("%s\t%s\t%s\t%s\t0\t%s\t%s\t%s\t0,0,0\t%s\t%s\t%s\t%s\n" %
                 (chr, items[2], items[3], items[10], items[1],
                  items[4], items[5], items[6], sizes_str, starts_str,
                  protname))
    gpf.close()
    bf.close()

def main():
    gpfile = sys.argv[1]
    bedfile = sys.argv[2]
    write_bed(gpfile, bedfile)

if __name__ == '__main__':
    main()
