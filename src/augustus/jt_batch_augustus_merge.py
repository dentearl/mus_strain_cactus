#!/usr/bin/env python
"""
jobTree based script that merges the results of the augustus gene
prediction pipeline.
"""
from argparse import ArgumentParser
from jobTree.scriptTree.stack import Stack
from jobTree.scriptTree.target import Target
from glob import glob
from math import floor
import os
from sonLib.bioio import logger
import subprocess
import sys
import time
sys.path.append(
  os.path.join(
    os.path.dirname(  # mus_strain_cactus
      os.path.dirname(  # src
        os.path.dirname(  # augustus
          os.path.abspath(sys.argv[0])))),
    'lib'))  # to import lib_run
import lib_run
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


class BatchJob(Target):
  """
  The BatchJob class runs the entire batch. It creates as many
  MergeCall() instances as required by the input.
  """
  def __init__(self, args):
    Target.__init__(self)
    self.args = args
  def run(self):
    count = 0
    windows = glob(os.path.join(self.args.in_dir, 'window_*', ''))
    seqs = set()
    for w in windows:
      gffs = glob(os.path.join(w, '*gff'))
    for g in gffs:
      seq = os.path.basename(g).split('.')[0]
      seqs.add(seq)
    count = len(seqs)
    [self.addChildTarget(MergeCall(windows, s, self.args)) for s in seqs]
    logger.debug('There will be %d MergeCall children' % count)
    self.args.batch_start_time = CreateSummaryReport(
      self.args.out_dir, self.args.batch_start_time, count,
      self.args.calling_command)


class MergeCall(Target):
  """
  MergeCall class merges all windows.
  """
  def __init__(self, dirs, seq, args):
    Target.__init__(self)
    self.dirs = dirs
    self.seq = seq
    self.args = args
  def run(self):
    comparative_cmds = [self.args.merger]
    single_cmds = [self.args.merger]
    comparative_gffs = []
    single_gffs = []
    comparative_outs = [os.path.join(self.args.out_dir,
                                     'merged.%s.cgp.gff' % self.seq)]
    single_outs = [os.path.join(
        self.args.out_dir, 'merged.%s.mea.gff' % self.seq)]
    comparative_errs = [os.path.join(self.args.out_dir,
                                     'merged.%s.stderr' % self.seq)]
    single_errs = [os.path.join(
      self.args.out_dir, 'merged.%s.mea.stderr' % self.seq)]
    for d in self.dirs:
      if os.path.exists(os.path.join(d, '%s.cgp.gff' % self.seq)):
        comparative_gffs.append(os.path.join(d, '%s.cgp.gff' % self.seq))
      if os.path.exists(os.path.join(d, '%s.mea.gff' % self.seq)):
        single_gffs.append(os.path.join(d, '%s.mea.gff' % self.seq))
    if comparative_gffs == [] and single_gffs == []:
      raise RuntimeError('Unable to find any input files!\n')
    comparative_cmds += comparative_gffs
    single_cmds += single_gffs
    if len(comparative_cmds) > 1:
      # comparative prediction mode
      time_start = lib_run.TimeStamp(
        self.args.out_dir, name='jt_issued_commands.%s.log' % self.seq)
      lib_run.LogCommand(self.args.out_dir, comparative_cmds,
                         name='jt_issued_commands.%s.log' % self.seq)
      lib_run.RunCommandsSerial([comparative_cmds], self.getLocalTempDir(),
                                out_pipes=comparative_outs,
                                err_pipes=comparative_errs)
      lib_run.TimeStamp(self.args.out_dir, time_start,
                        name='jt_issued_commands.%s.log' % self.seq)
    if len(single_cmds) > 1:
      # single prediction mode
      time_start = lib_run.TimeStamp(self.args.out_dir)
      lib_run.LogCommand(self.args.out_dir, single_cmds,
                         name='jt_issued_commands.%s.log' % self.seq)
      lib_run.RunCommandsSerial([single_cmds], self.getLocalTempDir(),
                                out_pipes=single_outs, err_pipes=single_errs)
      lib_run.TimeStamp(self.args.out_dir, time_start,
                        name='jt_issued_commands.%s.log' % self.seq)


def CreateSummaryReport(out_dir, now, count, command):
  """ Create a summary report in the root output directory.
  """
  f = open(os.path.join(out_dir, 'summary_report.txt'), 'w')
  f.write('run started: %s\n' % time.strftime("%a, %d %b %Y %H:%M:%S (%Z)",
                                              time.localtime(now)))
  f.write('command: %s\n' % command)
  f.write('num jobs:    %d\n' % count)
  f.close()
  return now


def InitializeArguments(parser):
  logger.debug('Initializing arguments')
  parser.add_argument('--in_dir', type=str,
                      help='location of augustus results directory.')
  parser.add_argument('--out_dir', type=str,
                      help='location to write out merged results.')
  parser.add_argument('--merger', type=str,
                      help='location of augustus_gff_merge.py.')


def CheckArguments(args, parser):
  # check for setting
  for name, value in [('in_dir', args.in_dir),
                      ('out_dir', args.out_dir),
                      ('merger', args.merger),
                      ]:
    if value is None:
      parser.error('Specify --%s' % name)
    else:
      value = os.path.abspath(value)
  # check for existence
  for name, value in [('in_dir', args.in_dir),
                      ('merger', args.merger),
                      ]:
    if not os.path.exists(value):
      parser.error('--%s %s does not exist' % (name, value))
  if not os.path.exists(args.out_dir):
    os.mkdir(args.out_dir)
  # check for directories
  for name, value in [('in_dir', args.in_dir),
                      ]:
    if not os.path.isdir(value):
      parser.error('--%s %s is not a directory' % (name, value))
  # check for execution
  # check for executability
  for value in [args.merger,
                ]:
    if not os.access(value, os.X_OK):
      parser.error('%s is not executable' % value)
  args.in_dir = os.path.abspath(args.in_dir)
  args.out_dir = os.path.abspath(args.out_dir)
  args.merger = os.path.abspath(args.merger)
  args.calling_command = '%s' % ' '.join(sys.argv[0:])


def LaunchBatch(args):
  args.batch_start_time = time.time()
  jobResult = Stack(BatchJob(args)).startJobTree(args)
  if jobResult:
    raise RuntimeError('The jobTree contained %d failed jobs!\n' % jobResult)
  f = open(os.path.join(args.out_dir, 'summary_report.txt'), 'a')
  now = time.time()
  f.write('run finished: %s\n' %
          time.strftime("%a, %d %b %Y %H:%M:%S (%Z)", time.localtime(now)))
  f.write('elapsed time: %s\n' % lib_run.PrettyTime(now -
                                                    args.batch_start_time))
  f.close()


def main():
  description = ('%(prog)s starts a jobTree batch of merges of augustus results'
                 ' for a given input set.')
  parser = ArgumentParser(description=description)
  InitializeArguments(parser)
  Stack.addJobTreeOptions(parser)
  args = parser.parse_args()
  CheckArguments(args, parser)

  LaunchBatch(args)


if __name__ == '__main__':
    from jt_batch_augustus_merge import *
    main()
