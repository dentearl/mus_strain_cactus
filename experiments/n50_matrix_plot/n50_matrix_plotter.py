#!/usr/bin/env python
"""
n50_matrix_plotter
4 sept 2014
dent earl, dearl a soe ucsc edu

tool to inspect data release directories and pull out *sizes files,
then create a matrix plot of species and n-curves.

"""
##############################
# plotting boilerplate / cargo cult
import matplotlib
matplotlib.use('Agg')
#####
# the param pdf.fonttype allows for text to be editable in Illustrator.
# Use either Output Type 3 (Type3) or Type 42 (TrueType)
matplotlib.rcParams['pdf.fonttype'] = 42
import matplotlib.backends.backend_pdf as pltBack
import matplotlib.lines as lines
import matplotlib.patches as patches
import matplotlib.pyplot as plt
import matplotlib.pylab as pylab
from matplotlib.ticker import ScalarFormatter, FormatStrFormatter
import numpy
##############################
from argparse import ArgumentParser
from glob import glob
from math import sqrt, floor, ceil
import os
import sys


class Size(object):
  def __init__(self, release, path):
    self.release = os.path.split(os.path.abspath(release))[1].split('_')[2]
    self.name = os.path.basename(path).split('.')[0]  # ./C57B6J.sizes -> C57B6J
    self.sizes = []
    with open(path, 'r') as f:
      for line in f:
        line = line.strip()
        self.sizes.append(int(line.split()[1]))
    self.normalizedSizes = []
    self.genomeSize = None
    self.cumSum = None
    self.calculateN50()
  def _sortSizes(self):
    self.sizes = numpy.sort(self.sizes)
    self.sizes = self.sizes[::-1]  # reverse
  def _normalizeSizes(self):
    self.sizes = numpy.array(self.sizes)
    self._sortSizes()
    if self.genomeSize is None:
      self.genomeSize = sum(self.sizes)
    self.normalizedSizes = [float(s) / self.genomeSize for s in self.sizes]
    self.normalizedSizes = numpy.array(self.normalizedSizes)
  def _calcCumSum(self):
    self.cumSum = numpy.cumsum(self.normalizedSizes, dtype=numpy.float64)
  def calculateN50(self):
    self._normalizeSizes()
    self._calcCumSum()
    # get the reverse index of the first value above 0.5
    ri = numpy.sum(self.cumSum > 0.5)
    if ri == 0:
      self.n50 = 0
    else:
      self.n50 = self.sizes[-ri]


def InitializeArguments(parser):
  parser.add_argument('releases', nargs='+', type=str,
                      help='assembly release directories.')
  parser.add_argument('--out', dest='out', default='my_plot',
                      type=str,
                      help=('path/filename where figure will be created. No '
                            'extension needed. default=%(default)s'))
  parser.add_argument('--height', dest='height', default=10, type=float,
                      help='height of image, in inches. default=%(default)s')
  parser.add_argument('--width', dest='width', default=11, type=float,
                      help='width of image, in inches. default=%(default)s')
  parser.add_argument('--dpi', dest='dpi', default=300,
                      type=int,
                      help=('dots per inch of raster outputs, i.e. '
                            'if --outFormat is all or png. '
                            'default=%(default)s'))
  parser.add_argument('--out_format', dest='out_format', default='pdf',
                      type=str,
                      help=('output format [pdf|png|eps|all]. '
                            'default=%(default)s'))
  parser.add_argument('--no_legend', dest='is_legend', default=True,
                      action='store_false',
                      help=('Turns off the filename / color legend. '
                            'Helpful for large numbers of files.'))
  parser.add_argument('--regression', dest='regression', default=False,
                      action='store_true',
                      help='turn on a simple linear regression line')
  parser.add_argument('--jitter', dest='jitter', default=False,
                      action='store_true',
                      help='turn on jitter for certain plotting modes')
  parser.add_argument('--random_seed', dest='random_seed', default=None,
                      type=int,
                      help=('Random seed for use with --jitter and '
                            '--downsample  flags.'))
  parser.add_argument('--aspect_equal', dest='aspect_equal', default=False,
                      action='store_true',
                      help='Turn on equal aspect ratio for the plot')


def CheckArguments(args, parser):
  if args.releases is None:
    parser.error('Specify *sizes files!')
  for d in args.releases:
    if not os.path.exists(d):
      parser.error('release directory %s does not exist.' % d)
    if not os.path.isdir(d):
      parser.error('%s is not a directory.' % d)


def InitImage(args):
  """ Initialize a new image.

  Args:
    args: an argparse arguments object

  Returns:
    fig: a matplotlib figure object
    pdf: a matplotlib pdf drawing (backend) object
  """
  pdf = None
  if args.out_format == 'pdf' or args.out_format == 'all':
    pdf = pltBack.PdfPages(args.out + '.pdf')
  fig = plt.figure(figsize=(args.width, args.height),
                   dpi=args.dpi, facecolor='w')
  return (fig, pdf)


def EstablishAxes(data_list, fig, args):
  """ Create a single axis on the figure object.

  Args:
    fig: a matplotlib figure object
    args: an argparse arguments object

  Returns:
    ax: a matplotlib axis object
  Raises:
    ValueError: If an unknown spine location is passed.
  """
  # left 0.99 inches, right 0.54 inches, width 7.47 inches
  # bottom 0.68 inches, top 0.28 inches, height 3.04 inches
  args.axLeft = 0.99 / args.width
  args.axRight = 1.0 - (0.54 / args.width)
  args.axWidth = args.axRight - args.axLeft
  args.axBottom = 0.68 / args.height
  args.axTop = 1.0 - (0.28 / args.height)
  args.axHeight = args.axTop - args.axBottom
  margin = 0.07
  ncol = floor(sqrt(len(data_list)))
  nrow = ceil(sqrt(len(data_list)))
  args.itemWidth = (args.axWidth - (ncol - 1) * margin) / float(ncol)
  args.itemHeight = (args.axHeight - (nrow - 1) * margin) / float(nrow)
  x, y = args.axLeft, args.axTop - args.itemHeight
  axDict = {}
  for i in xrange(0, len(data_list)):
    ax = fig.add_axes([x, y,
                       args.itemWidth, args.itemHeight])
    FixAxis(ax)
    axDict[data_list[i][0].name] = ax
    x += margin + args.itemWidth
    if not (1 + i) % ncol:
      x = args.axLeft
      y -= margin + args.itemHeight
  return axDict


def FixAxis(ax):
  ax.yaxis.set_major_locator(pylab.NullLocator())
  ax.xaxis.set_major_locator(pylab.NullLocator())
  for loc, spine in ax.spines.iteritems():
    if loc in ['left', 'bottom']:
      spine.set_position(('outward', 10))
    elif loc in ['right', 'top']:
      spine.set_color('none')
    else:
      raise ValueError('unknown spine location: %s' % loc)
  ax.xaxis.set_ticks_position('bottom')
  ax.yaxis.set_ticks_position('left')
  return ax


def WriteImage(fig, pdf, args):
  """ Write the image to disk.

  Args:
    fig: a matplotlib figure object
    pdf: a matplotlib pdf drawing (backend) object
    args: an argparse arguments object
  """
  if args.out_format == 'pdf':
    fig.savefig(pdf, format = 'pdf')
    pdf.close()
  elif args.out_format == 'png':
    fig.savefig(args.out + '.png', format='png', dpi=args.dpi)
  elif args.out_format == 'all':
    fig.savefig(pdf, format='pdf')
    pdf.close()
    fig.savefig(args.out + '.png', format='png', dpi=args.dpi)
    fig.savefig(args.out + '.eps', format='eps')
  elif args.out_format == 'eps':
    fig.savefig(args.out + '.eps', format='eps')


def ReadFiles(args):
  """
  """
  data = {}  # keyed by name, valued by lists of releases
  for r in args.releases:
    sizes = glob(os.path.join(r, '*sizes'))
    for n in sizes:
      s = Size(r, n)
      if s.name.startswith('Anc'):
        continue
      group = data.setdefault(s.name, [])
      group.append(s)
  groups = data.keys()
  group_pairs = []
  # go through the groups and sort them according to max n50 value
  for g in groups:
    group_pairs.append((g, max([s.n50 for s in data[g]])))  # tuple
  group_pairs = sorted(group_pairs, key=lambda g: g[1], reverse=True)
  # reorder the data according to group max n50
  data = [data[g[0]] for g in group_pairs]
  return data


def PlotData(data_list, axDict, args):
  for d in data_list:
    OnePlot(d, axDict[d[0].name], args)


def OnePlot(d_list, ax, args):
  """ Plot one axes worth of data. d is a list.
  """
  colors_medium = args.colors_medium = [( 98, 162, 209),  # m blue
                                        (190, 154,  87),  # m mustard
                                        (223, 133, 131),  # m pink
                                        (  0, 183, 134),  # m green
                                        (126, 173,  90),  # m olive
                                        (  0, 180, 181),  # m teal
                                        (187, 134, 209),  # m purple
                                        (225, 122, 179),  # m magenta
                                        ]
  for i in xrange(0, len(colors_medium)):
    colors_medium[i] = (colors_medium[i][0] / 255.0,
                        colors_medium[i][1] / 255.0,
                        colors_medium[i][2] / 255.0,)
  for i, d in enumerate(d_list):
    # plot each release for a given species
    ax.plot(d.cumSum, d.sizes, color=colors_medium[ReleaseToIndex(d.release)])
  ax.set_title(d.name)
  ax.set_xlim([0.0, 1.0])
  ax.set_xticks([0.0, 0.5, 1.0])
  ax.set_xticklabels([0, 0.5, 1.0])
  ax.locator_params(axis='y', nbins=4)
  ax.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
  ax.set_yscale('log')


def ReleaseToIndex(r):
  rti = {'1302' : 0,
         '1405' : 1,
         '1409' : 2,}
  assert r in rti
  return rti[r]


def main():
  parser = ArgumentParser()
  InitializeArguments(parser)
  args = parser.parse_args()
  CheckArguments(args, parser)

  data_list = ReadFiles(args)
  fig, pdf = InitImage(args)
  axDict = EstablishAxes(data_list, fig, args)

  PlotData(data_list, axDict, args)
  WriteImage(fig, pdf, args)


if __name__ == '__main__':
  main()
