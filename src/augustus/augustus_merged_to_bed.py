#!/usr/bin/env python
"""
18 august 2014
dent earl, dearl a soe ucsc edu

script to take the output of a augustus_gff_merge.py
and turn it into a bed based track for use with the
UCSC genome browser.
"""
from argparse import ArgumentParser
from glob import glob
import os


def InitializeArguments(parser):
  """
  """
  parser.add_argument('--in_dir', help='path to a merged augustus directory')
  parser.add_argument('--out_dir', help='path to output directory.')
  parser.add_argument('--track_prefix', help='preceedes \'_cgp\' and \'_mea\'')


def CheckArguments(args, parser):
  """
  """
  pairs = [(n, getattr(args, n)) for n in ['in_dir', 'out_dir', 'track_prefix']]
  for name, value in pairs:
    if value is None:
      parser.error('Specify --%s' % name)


def GetGffs(args):
  """ Return a tuple of lists of paths to gffs
  """
  # cgp = comparative genome prediction
  cgp = glob(os.path.join(args.in_dir,'*cgp.gff'))
  # mea = maximum expected accuracy
  mea = glob(os.path.join(args.in_dir,'*mea.gff'))
  return cgp, mea


def GffSpecies(g):
  """ given a path name to a gff, return the track species name.
  """
  tokens = g.split('.')
  assert tokens[-1] == 'gff'
  assert tokens[-2] in ['cgp', 'mea']
  s = tokens[-3]
  # this accounts for use cases where a single window
  # is used as in_dir instead of a merged result.
  s = os.path.basename(s)
  return s


def GffToName(g, mode):
  """ given a path name to an input gff, return an output bed name.
  """
  tokens = g.split('.')
  assert tokens[-1] == 'gff'
  assert tokens[-2] == mode
  s = tokens[-3]
  s = os.path.basename(s)
  return '%s.%s.bed' % (s, mode)


def MakeBedsBuildDirs(gffs, args):
  """ Build out the directory structure, convert gffs to beds.
  """
  if not os.path.exists(args.out_dir):
    os.mkdir(args.out_dir)
  cgp, mea = gffs
  for g in cgp:
    StartConversion(g, 'cgp', args)
  for g in mea:
    StartConversion(g, 'mea', args)


def StartConversion(g, mode, args):
  """ Given a gff file, make the directories and start the conversion process.
  """
  dir_prefix = os.path.join(args.out_dir, args.track_prefix + '_' + mode)
  if not os.path.exists(dir_prefix):
    os.mkdir(dir_prefix)
  if not os.path.exists(os.path.join(dir_prefix, GffSpecies(g))):
    os.mkdir(os.path.join(dir_prefix, GffSpecies(g)))
  GffToBed(g, os.path.join(dir_prefix, GffSpecies(g), GffToName(g, mode)))


def GffToBed(gff, bed):
  """ Given the input path to a GFF, convert it to a bed at path BED.
  """
  with open(bed, 'w') as b:
    with open(gff, 'r') as g:
      for line in g:
        line = line.strip()
        if line.startswith('#'):
          continue
        data = line.split()
        bedlist = [data[0], str(int(data[3]) - 1), data[4],
                   data[2], '0', data[6]]
        b.write('%s\n' % '\t'.join(bedlist))


def main():
  parser = ArgumentParser()
  InitializeArguments(parser)
  args = parser.parse_args()
  CheckArguments(args, parser)
  gffs = GetGffs(args)
  MakeBedsBuildDirs(gffs, args)


if __name__ == '__main__':
  main()
