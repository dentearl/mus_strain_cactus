
import os
import sys
import re

def write_bed(gpfile, bedfile):
    gpf = open(gpfile, 'r')
    bf = open(bedfile, 'w')
    for line in gpf:
        if re.search('name', line):
            continue
        items = line.strip("\n").split("\t")
        starts = [int(s) for s in items[8].rstrip(',').split(',')]
        ends = [int(e) for e in items[9].rstrip(',').split(',')]
        #convert starts & ends to be relative to genestart
        txstart = int(items[3])
        starts = [s - txstart for s in starts]
        ends = [e - txstart for e in ends]

        sizes = [e - starts[i] for i, e in enumerate(ends)]
        for i, start in enumerate(starts):
            assert start >= 0
            assert sizes[i] > 0

        chr = items[1].lstrip("chr")
        chritems = chr.split("_")
        if len(chritems) > 1:
            chr = chritems[1]
        if chr == 'M':
            continue
        starts_str = ",".join([str(start) for start in starts])
        sizes_str = ",".join([str(size) for size in sizes])
        bf.write("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t0,0,0\t%s\t%s\t%s\t%s\n" %
                 (chr, items[3], items[4], items[0], items[10], items[2],
                  items[5], items[6], items[7], sizes_str, starts_str,
                  items[11]))
    gpf.close()
    bf.close()

def main():
    gpfile = sys.argv[1]
    bedfile = sys.argv[2]
    write_bed(gpfile, bedfile)

if __name__ == '__main__':
    main()
