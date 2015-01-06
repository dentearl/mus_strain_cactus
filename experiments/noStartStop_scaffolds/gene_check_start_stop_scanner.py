#!/usr/bin/env python
"""
"""
from argparse import ArgumentParser
import os


def InitializeArguments(parser):
  parser.add_argument('gene_check', help='gene_check result file.')
  parser.add_argument('gene_check_details',
                      help='gene_check_details result file.')
  parser.add_argument('chrom_sizes',
                      help='file containing chromosome names and lengths.')
  parser.add_argument('--length_threshold', default=1000, type=int,
                      help=('distance threshold for ends of chromosomes. '
                            'default=%(default)s'))
  parser.add_argument('--print_distances', default=False, action='store_true',
                      help=('print out the distances to the nearest '
                            'scaffold edge'))


def CheckArguments(args, parser):
  # check for setting
  for name, value in [('gene_check', args.gene_check),
                      ('gene_check_details', args.gene_check_details),
                      ('chrom_sizes', args.chrom_sizes),
                      ]:
    if value is None:
      parser.error('Specify %s' % name)
    else:
      value = os.path.abspath(value)
  # check for path existence
  for name, value in [('gene_check', args.gene_check),
                      ('gene_check_details', args.gene_check_details),
                      ('chrom_sizes', args.chrom_sizes),
                      ]:
    if not os.path.exists(value):
      parser.error('--%s %s does not exist' % (name, value))
  # check for files
  for value in [args.gene_check, args.gene_check_details, args.chrom_sizes,
                ]:
    if not os.path.isfile(value):
      parser.error('%s is not a file' % value)


def ReadSizes(args):
  sizes = {}
  with open(args.chrom_sizes, 'r') as f:
    for line in f:
      line = line.strip()
      data = line.split()
      sizes[data[0]] = int(data[1])
  return sizes


def ReadStrands(args):
  strands = {}
  with open(args.gene_check, 'r') as f:
    for line in f:
      line = line.strip()
      data = line.split()
      if data[3] in strands:
        if data[5] != strands[data[3]]:
          raise RuntimeError('Duplicate entry found with differing '
                             'strands! %s: %s'
                             % (args.gene_check, data[3]))
      strands[data[0] + '_' + data[3]] = data[5]
  return strands


def ScanGeneCheckDetails(sizes, strands, args):
  if args.print_distances:
    ScanGeneCheckDetailsDistances(sizes, strands, args)
  else:
    ScanGeneCheckDetailsVanilla(sizes, strands, args)


def ScanGeneCheckDetailsDistances(sizes, strands, args):
  with open(args.gene_check_details, 'r') as f:
    for line in f:
      line = line.strip()
      data = line.split()
      chrom = data[0]
      values = data[3].split('/')
      error = values[0]
      entry = values[-1]
      if error not in ['noStart', 'noStop']:
        continue
      name = chrom + '_' + entry
      if strands[name] == '+':
        if error == 'noStop':
          print sizes[chrom] - int(data[2])
        else:
          print int(data[1])
      else:
        if error == 'noStart':
          print sizes[chrom] - int(data[2])
        else:
          print int(data[1])


def ScanGeneCheckDetailsVanilla(sizes, strands, args):
  with open(args.gene_check_details, 'r') as f:
    for line in f:
      line = line.strip()
      data = line.split()
      chrom = data[0]
      values = data[3].split('/')
      error = values[0]
      entry = values[-1]
      if error not in ['noStart', 'noStop']:
        continue
      name = chrom + '_' + entry
      if strands[name] == '+':
        if error == 'noStop':
          if sizes[chrom] - int(data[2]) < args.length_threshold:
            # too close to the end
            print line
        else:
          if int(data[1]) < args.length_threshold:
            # too close to the start
            print line
      else:
        if error == 'noStart':
          if sizes[chrom] - int(data[2]) < args.length_threshold:
            # too close to the end
            print line
        else:
          if int(data[1]) < args.length_threshold:
            # too close to the start
            print line


def main():
  parser = ArgumentParser()
  InitializeArguments(parser)
  args = parser.parse_args()
  CheckArguments(args, parser)
  sizes = ReadSizes(args)
  strands = ReadStrands(args)
  ScanGeneCheckDetails(sizes, strands, args)


if __name__ == '__main__':
  main()
