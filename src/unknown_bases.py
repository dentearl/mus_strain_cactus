import os
from src.abstract_classifier import AbstractClassifier
import lib.psl_genecheck_lib as psl_lib
import lib.sqlite_lib as sql_lib
from collections import Counter
from itertools import izip

class UnknownBases(AbstractClassifier):
    """
    Counts the number of Ns in the target sequence within alignment blocks

    """
    @staticmethod
    def __type__():
        return "INT"

    @staticmethod
    def __name__():
        return "UnknownBases"

    def run(self):
        #call methods from abstract_classifier that pull data needed for this classifier
        self.get_alignments()
        self.get_seq_dict()

        counts = Counter()
        for aln in self.alignments:
            for tStart, blockSize in izip(aln.tStarts, aln.blockSizes):
                seq = self.seq_dict[aln.tName].sliceSequence(tStart, tStart + blockSize)
                counts[aln.qName] += seq.count("N")

        with sql_lib.ExclusiveSqlConnection(self.output) as cur:
            for alignmentName, count in counts.iteritems():
                sql_lib.upsert(cur, self.genome, self.primary_key, alignmentName,
                        self.__name__(), count)