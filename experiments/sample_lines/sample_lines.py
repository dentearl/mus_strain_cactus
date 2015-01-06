#!/usr/bin/env python
"""
script to sample lines from a file.
may 29 2014
dent earl, dearl a soe ucsc edu

"""
import os
import random
from argparse import ArgumentParser


def InitializeArguments(parser):
  parser.add_argument('input', help='File to sample')
  parser.add_argument('-n', '--number_of_samples', type=int,
                      help='Number of lines to sample')


def CheckArguments(args, parser):
  if args.input is None:
    parser.error('Specify input file to sample as an argument.')
  if not os.path.exists(args.input):
    parser.error('Input file %s does not exist!' % args.input)
  if args.number_of_samples is None:
    parser.error('Specify --number_of_samples')


def GetFileLength(filename):
  i = 0
  with open(filename) as f:
    for i, line in enumerate(f, 1):
      pass
  return i


def PrintSamples(samples, args):
  with open(args.input) as f:
    for i, line in enumerate(f, 1):
      if samples is None:
        print line
      else:
        if i in samples:
          line = line.strip()
          print line


def main():
  parser = ArgumentParser()
  InitializeArguments(parser)
  args = parser.parse_args()
  CheckArguments(args, parser)
  length = GetFileLength(args.input)
  if not length:
    return
  if length < args.number_of_samples:
    PrintSamples(None, args)
  else:
    samples = random.sample(xrange(length), args.number_of_samples)
    PrintSamples(samples, args)


if __name__ == '__main__':
  main()
