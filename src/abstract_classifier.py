from jobTree.scriptTree.target import Target
from jobTree.src.bioio import logger
import lib.sequence_lib as seq_lib
import lib.psl_lib as psl_lib

class AbstractClassifier(Target):
    def __init__(self, genome, alnPsl, seqFasta, transcriptBed, outDb, refGenome, primaryKey):
        #initialize the Target
        Target.__init__(self)

        #store basic information
        self.genome = genome
        self.refGenome = refGenome
        self.output = outDb
        self.alnPsl = alnPsl
        self.seqFasta = seqFasta
        self.transcriptBed = transcriptBed
        self.primary_key = primaryKey
        self.db = outDb

    def get_transcripts(self):
        self.transcripts = seq_lib.getTranscripts(self.transcriptBed)

    def get_transcript_dict(self):
        if not hasattr(self, 'transcripts'):
            self.transcripts = seq_lib.getTranscripts(self.transcriptBed)
        self.transcript_dict = seq_lib.transcriptListToDict(self.transcripts, noDuplicates=True)

    def get_seq_dict(self):
        self.seq_dict = seq_lib.getFastaDict(self.seqFasta, upper=True)

    def get_alignments(self):
        self.alignments = psl_lib.readPsl(self.alnPsl, uniqify=True)

    def make_alignment_dict(self):
        if not hasattr(self, 'alignments'):
            self.get_alignments()
        self.alignment_dict = psl_lib.getPslDict(self.alignments)