import os
import argparse
import sqlite3 as sql
from jobTree.scriptTree.target import Target
from jobTree.scriptTree.stack import Stack
from jobTree.src.bioio import getLogLevelString, isNewer, logger, setLoggingFromOptions
from lib.sqlite_lib import initializeTable
from lib.general_lib import FileType, DirType, FullPaths
from src.unknown_bases import UnknownBases

#classifiers we are currently working with
classifiers = [UnknownBases]

#hard coded file extension types that we are looking for
alignment_ext = ".chained.psl"
sequence_ext = ".fa"

def build_parser():
    """
    Builds an argument parser for this run
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('--refGenome', type=str)
    parser.add_argument('--genomes', nargs="+")
    parser.add_argument('--annotationBed', type=FileType)
    parser.add_argument('--alignmentDir', type=DirType, action=FullPaths)
    parser.add_argument('--sequenceDir', type=DirType, action=FullPaths)
    parser.add_argument('--outDb', type=str, default="results.db")
    parser.add_argument('--primaryKey', type=str, default="AlignmentID")
    parser.add_argument('--overwriteDb', action="store_true")
    return parser


def parse_dir(genomes, targetDir, ext):
    """
    Given a directory, a dict of filetypes (mapping extension to name), and a list of genomes,
    returns a dict mapping all files matching those fileTypes and checks
    for existence of these files
    """
    pathDict = {}
    for g in genomes:
        path = os.path.join(targetDir, g + ext)
        if not os.path.exists(path):
            raise RuntimeError("{} does not exist in {}".format(f, targetDir))
        pathDict[g] = path
    return pathDict


def build_analysis(target, alnPslDict, seqFastaDict, genomes, annotationBed, outDb, primaryKey, refGenome):
    for genome in genomes:
        initialize_sql_table(genome, outDb, primaryKey)
        aln, seq = alnPslDict[genome], seqFastaDict[genome]
        for classifier in classifiers:
            target.addChildTarget(classifier(genome, aln, seq,
                    annotationBed, outDb, refGenome, primaryKey))


def initialize_sql_table(genome, outDb, primaryKey):
    con = sql.connect(outDb)
    columns = [[x.__name__, x.__type__()] for x in classifiers]
    with con:
        initializeTable(con.cursor(), genome, columns, primaryKey)


def main():
    parser = build_parser()
    Stack.addJobTreeOptions(parser)
    args = parser.parse_args()
    setLoggingFromOptions(args)

    if args.overwriteDb is True:
        os.remove(args.outDb)

    logger.info("Building paths to the required files")
    alnPslDict = parse_dir(args.genomes, args.alignmentDir, alignment_ext)
    seqFastaDict = parse_dir(args.genomes, args.sequenceDir, sequence_ext)

    refSequence = os.path.join(args.sequenceDir, args.refGenome + ".fa")
    if not os.path.exists(refSequence):
        raise RuntimeError("Reference genome fasta not present at {}".format(refSequence))
    args.refSequence = refSequence

    i = Stack(Target.makeTargetFn(build_analysis, args=(alnPslDict, seqFastaDict, args.genomes, 
            args.annotationBed, args.outDb, args.primaryKey, args.refGenome))).startJobTree(args)

    if i != 0:
        raise RuntimeError("Got failed jobs")



if __name__ == '__main__':
        main()