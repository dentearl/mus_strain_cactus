#!/usr/bin/env python
"""
augustus_batch_stats
14 July 2014

dent earl, dearl a soe ucsc edu

A script to walk over an output (even if it is in-progress) directory
of jt_batch_augustus.py and report back information on the amount of
time the different steps are taking / have taken.

"""
from argparse import ArgumentParser
from glob import glob
import os
import numpy
import re
import sys
sys.path.append(
  os.path.join(
    os.path.dirname(  # mus_strain_cactus
      os.path.dirname(  # src
        os.path.dirname(  # augustus
          os.path.abspath(sys.argv[0])))),
    'lib'))  # to import lib_run
import lib_run


class Window(object):
  def __init__(self):
    self.name = ''  # full path
    self.shortName = ''  # directory window
    self.halTime = None
    self.augTime = None
    self.cpTime = None
    self.inProgress = True
    self.halSuccess = False
    self.augSuccess = False
    self.cpSuccess = False
    self.halComplete = False
    self.augComplete = False
    self.cpComplete = False
    self.success = False


def initializeArguments(parser):
  parser.add_argument('--dir', type=str, help='augustus run directory.')
  parser.add_argument('--raw', default=False, action='store_true',
                      help='turn off human readable time formatting.')


def checkArguments(args, parser):
  pairs = tuple((item, getattr(args, item)) for item in
                ['dir',
                 ])
  # check for setting
  for name, value in pairs:
    if value is None:
      parser.error('Specify --%s' % name)
    else:
      value = os.path.abspath(value)
  # check for path existence
  for name, value in [('dir', args.dir),
                      ]:
    if not os.path.exists(value):
      parser.error('--%s %s does not exist' % (name, value))
    args.dir = os.path.abspath(args.dir)
  # check for directories
  for name, value in [('dir', args.dir),
                      ]:
    if not os.path.isdir(value):
      parser.error('--%s %s is not a directory' % (name, value))


def makeWindow(p):
  """ Given a path P to a window_* directory, return a Window object.
  """
  w = Window()
  w.name = p
  w.shortName = os.path.dirname(p)
  log = os.path.join(w.name, 'jt_issued_commands.log')
  if not os.path.exists(log):
    return None
  patElapsed = re.compile(r'^\[.+?\] End \(elapsed: (.+?)\)  # (.+?):(\w+)')
  with open(log, 'r') as f:
    w.inProgress = True
    for line in f:
      line = line.strip()
      m = patElapsed.search(line)
      if m is None:
        continue
      name, status = m.group(2), m.group(3)
      timeSeconds = lib_run.PrettyTimeToSeconds(m.group(1))
      if name == 'hal2maf':
        w.halComplete = True
        if status == 'success':
          w.halSuccess = True
        else:
          w.halSuccess = False
          w.inProgress = False
        w.halTime = timeSeconds
      elif name == 'augustus':
        w.augComplete = True
        if status == 'success':
          w.augSuccess = True
        else:
          w.halSuccess = False
          w.inProgress = False
        w.augTime = timeSeconds
      elif name == 'copy back':
        w.cpComplete = True
        if status == 'success':
          w.cpSuccess = True
        else:
          w.halSuccess = False
          w.inProgress = False
        w.cpTime = timeSeconds
  if w.halSuccess and w.augSuccess and w.cpSuccess:
    w.success = True
    w.inProgress = False
  return w


def getWindows(args):
  """ Get all of the window directories and return a list of Window objects
  """
  paths = glob(os.path.join(args.dir, 'window_*'))
  windows = [makeWindow(p) for p in paths]
  windows = [x for x in windows if x is not None]
  return windows


def formatTime(t, args):
  """ Return the time in the correct format.
  """
  if args.raw:
    return '%.1f' % t
  else:
    return lib_run.PrettyTime(t)


def reportResults(windows, args):
  """ Print everything out.
  """
  print('n: %d. successes: %d. failures: %d. in-progress: %d.' %
        (len(windows), len([w for w in windows if w.success]),
         len(windows) - len([w for w in windows if w.inProgress]) -
         len([w for w in windows if w.success]),
         len([w for w in windows if w.inProgress]),
         ))
  print('%9s  %18s % 18s %6s %18s %18s' %
        ('', 'ave', 'med', 'min', 'max', 'std'))
  for name, times in [
    ('hal2maf', [w.halTime for w in windows if w.halComplete]),
    ('augustus', [w.augTime for w in windows if w.augComplete]),
    ('copy', [w.cpTime for w in windows if w.cpComplete]),
    ]:
    if times == []:
      print '%s: --' % name
    else:
      tmean = formatTime(numpy.mean(times), args)
      tmed = formatTime(numpy.median(times), args)
      tmin = formatTime(numpy.min(times), args)
      tmax = formatTime(numpy.max(times), args)
      tstd = formatTime(numpy.std(times), args)
      print('%9s  %18s %18s %6s %18s %18s' %
            (name, tmean, tmed, tmin, tmax, tstd))


def main():
  parser = ArgumentParser()
  initializeArguments(parser)
  args = parser.parse_args()
  checkArguments(args, parser)
  windows = getWindows(args)
  reportResults(windows, args)


if __name__ == '__main__':
  main()
