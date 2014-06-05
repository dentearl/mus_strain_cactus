"""
convenience library for assisting filters.
"""
import os


class Sequence(object):
  """ Represents a sequence of DNA.
  """
  def __init__(self, name, sequence):
    self.name = name  # chromosome or scaffold name
    self._sequence = sequence  # ACGTs
  def setSequence(self, seq):
    self._sequence = seq
  def getSequence(self):
    return self._sequence
  def getLength(self):
    return len(self._sequence)


def InitializeArguments(parser):
  """ given an argparse ArgumentParser object, add in the default arguments.
  """
  parser.add_argument('--refGenome')
  parser.add_argument('--genome')
  parser.add_argument('--geneCheckBed')
  parser.add_argument('--geneCheckBedDetails')
  parser.add_argument('--alignment')
  parser.add_argument('--sequence')
  parser.add_argument('--chromSizes')
  parser.add_argument('--outDir')


def CheckArguments(args, parser):
  """ Make sure all of the args are properly set for the default arguments.
  """
  # setting
  pairs = tuple((item, getattr(args, item)) for item in
                ['refGenome', 'genome',
                 'geneCheckBed', 'geneCheckBedDetails',
                 'alignment', 'sequence', 'chromSizes',
                 'outDir'])
  for name, value in pairs:
    if value is None:
      parser.error('Specify --%s' % name)
  # existence
  pairs = tuple((item, getattr(args, item)) for item in
                ['geneCheckBed', 'geneCheckBedDetails',
                 'alignment', 'sequence', 'chromSizes',
                 'outDir'])
  for name, value in pairs:
    if not os.path.exists(value):
      parser.error('--%s=%s does not exist' % (name, value))
  # regular file
  pairs = tuple((item, getattr(args, item)) for item in
                ['geneCheckBed', 'geneCheckBedDetails',
                 'alignment', 'sequence', 'chromSizes',
                 ])
  for name, value in pairs:
    if not os.path.isfile(value):
      parser.error('--%s=%s is not a file' % (name, value))
  # directory
  if not os.path.isdir(args.outDir):
    parser.error('--outDir=%s is not a directory' % args.outDir)


def getSequences(seqFile):
  """ Given a path to a fasta file, return a dictionary of Sequence objects
  keyed on the sequence name
  """
  seqDict = {}
  seq = None
  with open(seqFile, 'r') as f:
    for seq in readSequence(f):
      seqDict[seq.name] = seq
  return seqDict


def readSequence(seqFile):
  """ provide an iterator that reads through fasta files.
  """
  buff = None
  eat_buffer = True
  while True:
    if eat_buffer:
      # new record
      if buff is None:
        header = ''
        while not header.startswith('>'):
          header = seqFile.readline().strip()
      else:
        header = buff
      assert(header.startswith('>'))
      name = header.replace('>', '').strip()
      seq = ''
    line = seqFile.readline().strip()
    if line:
      if line.startswith('>'):
        # stop processing the record, store this line.
        buff = line
        eat_buffer = True
        yield Sequence(name, seq)
      else:
        eat_buffer = False
        seq += line
    else:
      # eof
      if buff is not None:
        buff = None
        yield Sequence(name, seq)
      else:
        if seq != '':
          yield Sequence(name, seq)
          name = ''
          seq = ''
        else:
          return

"""The following data types are used for iterating over gene-check-detail and gene-check bed files.
An example of entries from such files:

[benedict@hgwdev tracks]$ more Rattus.coding.gene-check-details.bed
1       2812370 2812372 noStop/ENSMUST00000065527.4
1       2812370 2812372 noStop/ENSMUST00000095795.4

[benedict@hgwdev tracks]$ more Rattus.coding.gene-check.bed 
1       2812346 3113743 ENSMUST00000178026.1    0       -       2812370 3038729
128,0,0 9       54,2,89,249,90,165,105,13,45    0,58,62,698,1209,1305,226292,301
050,301352
1       2812346 3113783 ENSMUST00000095795.4    0       -       2812370 3038729
128,0,0 9       54,2,89,249,197,52,105,13,85    0,58,62,698,1209,1418,226292,301
050,301352
"""

class ChromosomeInterval:
    """Represents an interval of a chromosome. BED coordinates, strand is True, False or NULL (if no strand)
    """
    def __init__(self, chromosome, start, stop, strand):
        self.chromosome = str(chromosome)
        self.start = int(start)
        self.stop = int(stop)
        self.strand = strand
    
class TranscriptAnnotation: 
    """Represents an annotation of a transcript, from one of the classification bed files
    """
    def __init__(self, chromosomeInterval, transcript, annotation):
        self.chromosomeInterval = chromosomeInterval
        self.transcript = str(transcript)
        self.annotation = annotation

class Transcript: 
    """Represent a transcript and its annotations
    """
    def __init__(self, chromosomeInterval, transcript, exons, annotations):
        self.chromosomeInterval = chromosomeInterval
        self.transcript = str(transcript)
        self.exons = exons #Is a list of chromosome intervals
        self.annotations = annotations #Is a list of transcript annotations

def tokenizeBedFile(bedFile):
    """Iterator through bed file, returning lines as list of tokens
    """
    fileHandle = open(bedFile, 'r')
    for line in fileHandle:
        if line != '':
            tokens = line.split()
            yield tokens
    fileHandle.close()

def transcriptIterator(transcriptsBedFile, transcriptClassificationBedFile):
    """Iterates over the transcripts detailed in the two files, producing Transcript objects.
    """
    transcriptsAnnotations = {}
    for bedTokens in tokenizeBedFile(transcriptClassificationBedFile):
        assert len(bedTokens) == 5 #We expect to be able to get 5 fields out of this bed.
        tA = TranscriptAnnotation(ChromosomeInterval(tokens[0], tokens[1], tokens[2], None), tokens[3], tokens[4])
        if tA.transcript not in transcriptsClassifications:
            transcriptsAnnotations[tA.transcript] = []
        transcriptsAnnotations[tA.transcript].append(tA)
    
    for bedTokens in tokenizeBedFile(transcriptsBedFile):
        assert len(bedTokens) == 12 #We expect to be able to get 12 fields out of this bed.
        #Transcript
        transcript = tokens[3]
        #Get the chromosome interval
        assert tokens[5] in ('+', '-')
        cI = ChromosomeInterval(tokens[0], tokens[1], tokens[2], tokens[5] == '+')
        #Get the exons
        def getExons(exonNumber, blockSizes, blockStarts):
            assert exonNumber == len(blockSizes)
            assert exonNumber == len(blockStarts)
            return [ ChromosomeInterval(cI.chromosome, cI.start + int(blockStarts[i]), cI.start + int(blockStarts[i]) + int(blockSizes[i]), cI.strand) \
                    for i in range(exonNumber) ]
        exons = getExons(int(tokens[9]), ",".split(tokens[10]), ",".split(tokens[11]))
        #Get the transcript annotations
        annotations = []
        if transcript in transcriptsAnnotations:
            annotations = transcriptsAnnotations[transcript]
        yield Transcript(chromosomeInterval, transcript, exons, annotations)
        

