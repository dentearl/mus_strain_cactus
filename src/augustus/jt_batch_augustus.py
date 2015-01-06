#!/usr/bin/env python
"""
jobTree based script that runs the augustus gene prediction pipeline.
"""
from argparse import ArgumentParser
from jobTree.scriptTree.stack import Stack
from jobTree.scriptTree.target import Target
from glob import glob
import os
import sys
sys.path.append(
  os.path.join(
    os.path.dirname(  # mus_strain_cactus
      os.path.dirname(  # src
        os.path.dirname(  # augustus
          os.path.abspath(sys.argv[0])))),
    'lib'))  # to import lib_run
import lib_run
# import MySQLdb
from sonLib.bioio import logger
import subprocess
import time
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
  AugustusCall() instances as required by the input.
  """
  def __init__(self, args):
    Target.__init__(self)
    self.args = args
  def run(self):
    sequence_dict = self.collect_sequences()
    debug('%s\n' % str(sequence_dict), self.args)
    order = sorted(sequence_dict.keys())
    for s in order:
      count = 0
      window_start, window_end = sequence_dict[s]
      for start in xrange(window_start, window_end,
                          self.args.window_length - self.args.window_overlap):
        count += 1
        actual_length = min(window_end - start, self.args.window_length)
        debug('AugustusCall(%s, %s, %s, %d, %d, %d, args)\n' %
              (self.args.hal_file_path, self.args.ref_genome, s,
               start, actual_length, count - 1), self.args)
        self.addChildTarget(AugustusCall(self.args.hal_file_path,
                                         self.args.ref_genome,
                                         s, start,
                                         actual_length,
                                         count - 1, self.args))
    debug('There will be %d AugustusCall children\n' % count, self.args)
    self.args.batch_start_time = createSummaryReport(
      self.args.out_dir, self.args.ref_genome, self.args.ref_sequence,
      self.args.window_start, actual_length, self.args.window_overlap,
      count, self.args.batch_start_time,
      self.args.calling_command)
  def collect_sequences(self):
    if self.args.ref_sequence is None:
      # handle this by collecting all sequences via a call to
      # halStats --sequenceStats
      debug('ref_sequence is none\n', self.args)
      return self.run_hal_stats()
    else:
      if self.args.window_end is None:
        # get the end from halStats
        debug('window_end is none\n', self.args)
        return self.run_hal_stats()
      else:
        return {self.args.ref_sequence:
                  (self.args.window_start, self.args.window_end)}
  def run_hal_stats(self):
    c = [os.path.join(self.args.hal_path, 'bin', 'halStats'),
         self.args.hal_file_path, '--sequenceStats',
         self.args.ref_genome]
    p = subprocess.Popen(c,
                         cwd=self.getLocalTempDir(),
                         stdout=subprocess.PIPE)
    p_out, p_err = p.communicate()
    if p_err is not None:
      raise RuntimeError('Error returned from halStats: %s\n' % p_err)
    p_list = p_out.split('\n')
    seqs_dict = {}
    for i, line in enumerate(p_list, 0):
      if i == 0:
        # header
        continue
      line = line.strip()
      if line == '':
        continue
      d = line.split(',')
      if self.args.ref_sequence is not None:
        if d[0] == self.args.ref_sequence:
          seqs_dict[d[0]] = (0, int(d[1]))
      else:
        seqs_dict[d[0]] = (0, int(d[1]))
    return seqs_dict


class AugustusCall(Target):
  """
  AugustusCall class calls the augustus binary.
  """
  def __init__(self, hal_file_path, ref_genome, ref_sequence,
               window_start, window_length,
               window_number, args):
    Target.__init__(self)
    self.hal_file_path = hal_file_path
    self.ref_genome = ref_genome
    self.ref_sequence = ref_sequence
    self.window_start = window_start
    self.window_length = window_length
    self.window_number = window_number
    self.out_path = os.path.join(args.out_dir,
                                 'window_%s_%s_%04d'
                                 % (ref_genome, ref_sequence,
                                    window_number))
    self.args = args
    # dbaccess = readDBAccess(self.args.dbaccess_file)
    self.aug_parameters = {
      'AUGUSTUS_CONFIG_PATH': os.path.join(self.args.augustus_path, 'config'),
      'treefile': self.args.tree_path,
      'species': self.args.species,
      'dbaccess': self.args.sqlite_db,
      'speciesfilenames': self.args.speciesfilenames,
      'softmasking': self.args.softmasking,
      'temperature': self.args.temperature,
      '/MeaPrediction/x0_E': self.args._MeaPrediction_x0_E,
      '/MeaPrediction/x0_I': self.args._MeaPrediction_x0_I,
      '/MeaPrediction/x1_E': self.args._MeaPrediction_x1_E,
      '/MeaPrediction/x1_I': self.args._MeaPrediction_x1_I,
      '/MeaPrediction/y0_E': self.args._MeaPrediction_y0_E,
      '/MeaPrediction/y0_I': self.args._MeaPrediction_y0_I,
      '/MeaPrediction/alpha_E': self.args._MeaPrediction_alpha_E,
      '/MeaPrediction/alpha_I': self.args._MeaPrediction_alpha_I,
      '/MeaPrediction/i1_E': self.args._MeaPrediction_i1_E,
      '/MeaPrediction/i1_I': self.args._MeaPrediction_i1_I,
      '/MeaPrediction/i2_E': self.args._MeaPrediction_i2_E,
      '/MeaPrediction/i2_I': self.args._MeaPrediction_i2_I,
      '/MeaPrediction/j1_E': self.args._MeaPrediction_j1_E,
      '/MeaPrediction/j1_I': self.args._MeaPrediction_j1_I,
      '/MeaPrediction/j2_E': self.args._MeaPrediction_j2_E,
      '/MeaPrediction/j2_I': self.args._MeaPrediction_j2_I,
      '/CompPred/ec_score': self.args._CompPred_ec_score,
      '/CompPred/ec_addend': self.args._CompPred_ec_addend,
      '/CompPred/ec_factor': self.args._CompPred_ec_factor,
      '/CompPred/dd_factor': self.args._CompPred_dd_factor,
      '/CompPred/exon_gain': self.args._CompPred_exon_gain,
      '/CompPred/exon_loss': self.args._CompPred_exon_loss,
      '/CompPred/phylo_factor': self.args._CompPred_phylo_factor,
      }
    if self.args._CompPred_only_species is not None:
      self.aug_parameters['/CompPred/only_species'] = (
        self.args._CompPred_only_species)
    if self.args.extrinsicCfgFile is not None:
      self.aug_parameters['extrinsicCfgFile'] = (
        self.args.extrinsicCfgFile)
      self.aug_parameters['dbhints'] = 'true'

  def run(self):
    if not os.path.exists(self.out_path):
      os.mkdir(self.out_path)
    if self.args.maf_file_path is None:
      self.maf_file = os.path.join(self.getLocalTempDir(),
                                   'window_%s_%s_%04d.maf'
                                   % (self.ref_genome,
                                      self.ref_sequence,
                                      self.window_number))
    else:
      self.maf_file = self.args.maf_file_path
    # verifyMySQLServer(self.out_path, self.args)  # Verify for nodes
    self.aug_parameters['alnfile'] = self.maf_file
    # extract the region needed as maf
    hal2maf_cmd = [os.path.join(self.args.hal_path, 'bin', 'hal2maf')]
    hal2maf_cmd.append('--refGenome')
    hal2maf_cmd.append(self.ref_genome)
    hal2maf_cmd.append('--noAncestors')  # Augustus throws these out.
    hal2maf_cmd.append('--refSequence')
    hal2maf_cmd.append(self.ref_sequence)
    hal2maf_cmd.append('--start')
    hal2maf_cmd.append(str(self.window_start))
    hal2maf_cmd.append('--length')
    hal2maf_cmd.append(str(self.window_length))
    hal2maf_cmd.append(self.args.hal_file_path)
    hal2maf_cmd.append(self.maf_file)
    hal2maf_cmds = [hal2maf_cmd]
    hal_err_pipe = [os.path.join(self.out_path, 'stderr.hal.out')]
    hal_out_pipe = [os.path.join(self.out_path, 'stdout.hal.out')]
    if self.args.maf_file_path is None:
      self.run_command_list(hal2maf_cmds, hal_out_pipe,
                            hal_err_pipe, tag='hal2maf')
    # run augustus on the maf
    aug_err_pipe = [os.path.join(self.out_path, 'stderr.aug.out')]
    aug_out_pipe = [os.path.join(self.out_path, 'stdout.aug.out')]
    aug_cmd = [os.path.join(self.args.augustus_path, 'bin', 'augustus')]
    for key in self.aug_parameters:
      aug_cmd.append('--%s=%s' % (key, str(self.aug_parameters[key])))
    aug_cmds = [aug_cmd]
    self.run_command_list(aug_cmds, aug_out_pipe, aug_err_pipe, tag='augustus')
    # copy output files from tmp back to the target dir
    copy_cmds = []
    # todo: copy out actual results
    for suffix in ['dot', 'gff', 'gff3', 'out', 'wig']:
      files = glob(os.path.join(self.getLocalTempDir(), '*.%s' % suffix))
      for f in files:
        copy_cmds.append([lib_run.Which('cp'), f, os.path.join(self.out_path)])
    self.run_command_list(copy_cmds, tag='copy back')

  def run_command_list(self, cmd_list, out_pipes=None, err_pipes=None, tag=''):
    """ Run a command list, log the commands, record timestamps before & after.
    """
    time_start = lib_run.TimeStamp(self.out_path, tag=tag)
    lib_run.LogCommand(self.out_path, cmd_list, out_pipe=out_pipes,
                       err_pipe=err_pipes)
    if not self.args.debug:
      try:
        lib_run.RunCommandsSerial(cmd_list, self.getLocalTempDir(),
                                  out_pipes=out_pipes, err_pipes=err_pipes)
        tag += ':success'
      except:
        tag += ':failure'
        raise
      finally:
        lib_run.TimeStamp(self.out_path, time_start, tag=tag)
    else:
      tag += ':debug'
      lib_run.TimeStamp(self.out_path, time_start, tag=tag)


def debug(s, args):
  if not args.debug:
    return
  f = open(os.path.join(args.out_dir, 'debugging.txt'), 'a')
  f.write('[debug] %s' % s)
  f.close()


def readDBAccess(dbaccess_file):
  """ Open the file and read the first line, report it back.
  """
  f = open(dbaccess_file, 'r')
  for line in f:
    if line.startswith('#'):
      continue
    line = line.strip()
    return line


def createSummaryReport(out_dir, ref_genome, ref_sequence, window_start,
                        window_length, window_overlap, count,
                        now, command):
  """ Create a summary report in the root output directory.
  """
  f = open(os.path.join(out_dir, 'jt_summary_report.txt'), 'w')
  f.write('run started: %s\n' % time.strftime("%a, %d %b %Y %H:%M:%S (%Z)",
                                              time.localtime(now)))
  f.write('command: %s\n' % command)
  f.write('window start:   %s\n' % str(window_start))  # can be None or int
  f.write('window length:  %d\n' % window_length)
  f.write('window overlap: %d\n' % window_overlap)
  f.write('num windows:    %d\n' % count)
  f.close()
  return now


def initializeArguments(parser):
  logger.debug('Initializing arguments')
  parser.add_argument('--augustus_path', type=str,
                      help='location of augustus directory.')
  parser.add_argument('--hal_path', type=str,
                      help='location of hal tools directory.')
  parser.add_argument('--hal_file_path', type=str,
                      help='location hal file.')
  parser.add_argument('--tree_path', type=str,
                      help='location newick tree file.')
  parser.add_argument('--out_dir', type=str,
                      help='location to store output files.')
  parser.add_argument('--sqlite_db', type=str,
                      help='location of sqlite database.')
  parser.add_argument(
    '--softmasking', type=str, choices=['true', 'false'], default='true',
    help='penalize exons in softmasked regions. default=%(default)s')
  parser.add_argument('--extrinsicCfgFile', help='extrinsic hint config file.')
  parser.add_argument('--speciesfilenames', type=str,
                      help=('location of the species file (text, one line per '
                            'species and location of .fa.'))
  # parser.add_argument('--dbaccess_file', type=str,
  #                     help='location of dbaccess file containing login info.')
  parser.add_argument('--maf_file_path', type=str,
                      help=('location maf file. Overrides all hal window '
                            'extraction. Debugging feature.'))
  parser.add_argument('--debug', default=False, action='store_true',
                      help='turns off execution of commands, just writes logs.')
  window = parser.add_argument_group('Window options')
  window.add_argument('--ref_genome', type=str,
                      help='reference genome to use for region extraction.')
  window.add_argument('--ref_sequence', type=str,
                      help=('reference sequence (chr) to use for '
                            'region extraction.'))
  window.add_argument('--window_start', type=int, default=0,
                      help='start of windowing region.')
  window.add_argument('--window_end', type=int, default=None,
                      help='end of windowing region.')
  window.add_argument('--window_length', type=int, default=2000000,
                      help='length of each window. default=%(default)s')
  window.add_argument('--window_overlap', type=int, default=1000000,
                      help='overlap of each window. default=%(default)s')
  augustus = parser.add_argument_group('Augustus options')
  augustus.add_argument('--species', default='human', type=str,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--temperature', default=3, type=int,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/x0_E', default=-1.25,
                        dest='_MeaPrediction_x0_E', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/x0_I', default=-0.78125,
                        dest='_MeaPrediction_x0_I', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/x1_E', default=5,
                        dest='_MeaPrediction_x1_E', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/x1_I', default=10,
                        dest='_MeaPrediction_x1_I', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/y0_E', default=0.5,
                        dest='_MeaPrediction_y0_E', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/y0_I', default=0.9,
                        dest='_MeaPrediction_y0_I', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/alpha_E', default=9.375,
                        dest='_MeaPrediction_alpha_E', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/alpha_I', default=2.5075,
                        dest='_MeaPrediction_alpha_I', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/i1_E', default=0.,
                        dest='_MeaPrediction_i1_E', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/i1_I', default=0.,
                        dest='_MeaPrediction_i1_I', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/i2_E', default=0.5,
                        dest='_MeaPrediction_i2_E', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/i2_I', default=0.9,
                        dest='_MeaPrediction_i2_I', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/j1_E', default=-1.25,
                        dest='_MeaPrediction_j1_E', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/j1_I', default=-0.78125,
                        dest='_MeaPrediction_j1_I', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/j2_E', default=0.,
                        dest='_MeaPrediction_j2_E', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/MeaPrediction/j2_I', default=0.,
                        dest='_MeaPrediction_j2_I', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/CompPred/ec_score', default=-13,
                        dest='_CompPred_ec_score', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/CompPred/ec_addend', default=-20.6,
                        dest='_CompPred_ec_addend', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/CompPred/ec_factor', default=6,
                        dest='_CompPred_ec_factor', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/CompPred/dd_factor', default=20,
                        dest='_CompPred_dd_factor', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/CompPred/exon_gain', default=0.0001,
                        dest='_CompPred_exon_gain', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/CompPred/exon_loss', default=0.0001,
                        dest='_CompPred_exon_loss', type=float,
                        help='Augustus option. default=%(default)s')
  augustus.add_argument('--/CompPred/phylo_factor', default=50.,
                        dest='_CompPred_phylo_factor', type=float,
                        help='Augustus option. default=%(default)s')
  #####
  augustus.add_argument('--/CompPred/only_species', default=None,
                        dest='_CompPred_only_species', type=str,
                        help='Augustus option. default=%(default)s')


def checkArguments(args, parser):
  # check for setting
  for name, value in [('augustus_path', args.augustus_path),
                      ('hal_path', args.hal_path),
                      ('hal_file_path', args.hal_path),
                      ('tree_path', args.tree_path),
                      ('out_dir', args.out_dir),
                      ('sqlite_db', args.sqlite_db),
                      ('speciesfilenames', args.speciesfilenames),
                      ('ref_genome', args.ref_genome),
                      ]:
    if value is None:
      parser.error('Specify --%s' % name)
    else:
      value = os.path.abspath(value)
  # check for path existence
  for name, value in [('augustus_path', args.augustus_path),
                      ('hal_path', args.hal_path),
                      ('hal_file_path', args.hal_file_path),
                      ('tree_path', args.tree_path),
                      ('sqlite_db', args.sqlite_db),
                      ('speciesfilenames', args.speciesfilenames),
                      ]:
    if not os.path.exists(value):
      parser.error('--%s %s does not exist' % (name, value))
  if not os.path.exists(args.out_dir):
    os.mkdir(args.out_dir)
  # check for directories
  for name, value in [('augustus_path', args.augustus_path),
                      ('hal_path', args.hal_path),
                      ('out_dir', args.out_dir)]:
    if not os.path.isdir(value):
      parser.error('--%s %s is not a directory' % (name, value))
  # check for files
  for value in [os.path.join(args.augustus_path, 'bin', 'augustus'),
                os.path.join(args.hal_path, 'bin', 'hal2maf'),
                args.hal_file_path,
                args.tree_path,
                args.sqlite_db,
                args.speciesfilenames,
                ]:
    if not os.path.isfile(value):
      parser.error('%s is not a file' % value)
  # check for executability
  for value in [(os.path.join(args.augustus_path, 'bin', 'augustus')),
                      (os.path.join(args.hal_path, 'bin', 'hal2maf'))]:
    if not os.access(value, os.X_OK):
      parser.error('%s is not executable' % value)
  if args.extrinsicCfgFile is not None:
    if not os.path.exists(args.extrinsicCfgFile):
      parser.error('--extrinsicCfgFile %s does not exist'
                   % args.extrinsicCfgFile)
    if not os.path.isfile(args.extrinsicCfgFile):
      parser.error('--extrinsicCfgFile %s is not a file'
                   % args.extrinsicCfgFile)
    args.extrinsicCfgFile = os.path.abspath(args.extrinsicCfgFile)
  args.augustus_path = os.path.abspath(args.augustus_path)
  args.hal_path = os.path.abspath(args.hal_path)
  args.hal_file_path = os.path.abspath(args.hal_file_path)
  args.tree_path = os.path.abspath(args.tree_path)
  args.out_dir = os.path.abspath(args.out_dir)
  if args.window_length - args.window_overlap < 1:
    parser.error('--window_length must be greater than --window_overlap!')
  # verifyMySQLServer(args.out_dir, args)  # Verify for head node
  logger.debug('Arguments checked.\n'
               'augustus_path:%s\n'
               'hal_path:%s\n'
               'hal_file_path:%s\n'
               'tree_path:%s\n'
               'out_dir:%s\n'
               % (args.augustus_path, args.hal_path, args.hal_file_path,
                  args.tree_path, args.out_dir))
  args.calling_command = '%s' % ' '.join(sys.argv[0:])
  if args.maf_file_path is not None:
    if args.ref_sequence is None:
      parser.error('You have selected --maf_file_path, you must also specify '
                   '--ref_sequence')
    if args.window_start is None:
      parser.error('You have selected --maf_file_path, you must also specify '
                   '--window_start')
    if args.window_end is None:
      parser.error('You have selected --maf_file_path, you must also specify '
                   '--window_end')


def verifyMySQLServer(out_dir, args):
  """ Make sure the MySQL server exists and is accessible.
  """
  def simple_connection_test(host_name, user_name, password, db_name, f):
    db = MySQLdb.connect(host=host_name, user=user_name,
                         passwd=password, db=db_name)
    cur = db.cursor()
    try:
      cur.execute('SELECT * FROM speciesnames '
                  'WHERE speciesname = "C56B6NJ" '
                  'LIMIT 10')
      rows = cur.fetchall()
    except MySQLdb.Error, e:
      f.write('[%s] MySQL Error [%d]: %s\n'
              % (lib_run.TimeString(), e.args[0], e.args[1]))
    cur.close()
    db.close()
  dbaccess = readDBAccess(args.dbaccess_file)
  db_name, host_name, user_name, password = dbaccess.split(',')
  f = open(os.path.join(out_dir, 'jt_db_check.log'), 'w')
  then = time.time()
  f.write('[%s] Checking host:%s database:%s user:%s pass:********\n'
          % (lib_run.TimeString(then), host_name, db_name, user_name))
  try:
    simple_connection_test(host_name, user_name, password, db_name, f)
  except:
    e = sys.exc_info()[0]
    f.write('[%s] exception caught: %s.\n' % (lib_run.TimeString(), e))
    raise
  else:
    f.write('[%s] db okay.\n' % lib_run.TimeString())
  finally:
    now = time.time()
    elapsed_time = now - then
    f.write('[%s] End (elapsed: %s)\n'
          % (lib_run.TimeString(), lib_run.PrettyTime(elapsed_time)))
  f.close()


def launchBatch(args):
  args.batch_start_time = time.time()
  jobResult = Stack(BatchJob(args)).startJobTree(args)
  if jobResult:
    raise RuntimeError('The jobTree contained %d failed jobs!\n' % jobResult)
  f = open(os.path.join(args.out_dir, 'jt_summary_report.txt'), 'a')
  now = time.time()
  f.write('run finished: %s\n' %
          time.strftime("%a, %d %b %Y %H:%M:%S (%Z)", time.localtime(now)))
  f.write('elapsed time: %s\n' % lib_run.PrettyTime(now -
                                                    args.batch_start_time))
  f.close()


def main():
  description = ('%(prog)s starts a jobTree batch of augustus calls '
                 'for a given input set.')
  parser = ArgumentParser(description=description)
  initializeArguments(parser)
  Stack.addJobTreeOptions(parser)
  args = parser.parse_args()
  checkArguments(args, parser)

  launchBatch(args)


if __name__ == '__main__':
    from jt_batch_augustus import *
    main()
