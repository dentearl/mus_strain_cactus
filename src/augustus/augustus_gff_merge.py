#!/usr/bin/env python
"""
may 2014
dent earl dearl a soe ucsc edu

Tool to merge, and sort gff output of Augustus gene prediction software
(and to remove duplicate entries).
"""
from argparse import ArgumentParser
import os
import sys
##################################################
# Copyright (c) 2014 Dent Earl, Benedict Paten, Mark Diekhans, Craig Hunter
# ... and other members of the Reconstruction Team of David Haussler's
# lab (BME Dept. UCSC)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
##################################################


class GffBlock(object):
  """
  """
  def __init__(self, header_line, window_id):
    self.window_id = window_id
    self.lines = []
    self.seqnames = set()
    self.features = set()
    self.start = -1
    self.end = -1
    self.original_name = header_line.replace('# start gene ', '')
    self.name = '%s_%s' % (window_id, self.original_name)  # unique in output
  def add_gffline(self, g):
    if self.lines == []:
      self.start = g.start
      self.end = g.end
    self.seqnames.add(g.seqname)
    self.features.add(g.feature)
    g.group = g.group.replace(self.original_name, self.name)
    if self.start > g.start:
      self.start = g.start
    if self.end < g.end:
      self.end = g.end
    self.lines.append(g)


class GffLine(object):
  """
  """
  def __init__(self, line):
    self.line = line
    data = line.split('\t')
    assert(len(data) == 9)
    self._id_string = ''.join(data[0:5] + data[6:8])  # checking uniqueness
    self.seqname = data[0]
    self.source = data[1]
    self.feature = data[2]
    self.start = int(data[3])  # 1 based
    self.end = int(data[4])  # inclusive
    self.score = data[5]
    self.strand = data[6]
    self.frame = data[7]
    self.group = data[8]
  def __str__(self):
    return('%s\t%s\t%s\t%d\t%d\t%s\t%s\t%s\t%s' %
           (self.seqname, self.source, self.feature,
            self.start, self.end, self.score,
            self.strand, self.frame, self.group))


def InitializeArguments(parser):
  parser.add_argument('gffs', nargs='+', type=str, help='input gff files')
  parser.add_argument('--out_dir', type=str,
                      help='location to write out merged results.')


def CheckArguments(args, parser):
  # check for existence
  for value in args.gffs:
    if not os.path.exists(value):
      parser.error('%s does not exist' % (value))


def ReadData(args):
  """ Read all the data from different windows.
  """
  data = []
  data_ids = set()
  for f in args.gffs:
    window_id = os.path.basename(os.path.dirname(f))
    data += ReadDataFile(f, data_ids, window_id, args)
  return data


def ReadDataFile(filename, data_ids, window_id, args):
  """ Read data from filename and produce de-duplicated unsorted collection.
  """
  data = []
  f = open(filename, 'r')
  for line in f:
    line = line.strip()
    if line.startswith('#'):
      if line.startswith('# start '):
        b = GffBlock(line, window_id)
        data.append(b)
        continue
      else:
        continue
    g = GffLine(line)
    if g._id_string not in data_ids:
      data_ids.add(g._id_string)
      b.add_gffline(g)
    else:
      sys.stderr.write('Removing duplicate from %s: %s\n' % (window_id, g))
  return data


def SortData(data, args):
  result = []
  map(result.extend, map(lambda b: b.lines, data))
  data = result
  data = sorted(data, key=lambda g: g.seqname)
  data = sorted(data, key=lambda g: g.feature)
  data = sorted(data, key=lambda g: g.start)
  return data


def ReportData(data, args):
  for g in data:
    print g


def main():
  description = ('%(prog)s takes gff data via stdin and outputs sorted, de-'
                 'duplicated data via stdout. Gff data is assumed to be from '
                 'AUGUSTUS; all of the group field entries are adjusted.')
  parser = ArgumentParser(description=description)
  InitializeArguments(parser)
  args = parser.parse_args()
  CheckArguments(args, parser)
  data = ReadData(args)
  data = SortData(data, args)
  ReportData(data, args)


if __name__ == '__main__':
  main()
