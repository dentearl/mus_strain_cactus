#!/usr/bin/env python
"""
stat_tag_matrix_plotter
5 sept 2014
dent earl, dearl a soe ucsc edu

tool to inspect paired pipeline result directories and pull out stat.xml files
then create a matrix plot of the deltas in different tags.

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
sys.path.append(
  os.path.join(
    os.path.dirname(  # mus_strain_cactus
      os.path.dirname(  # experiments
        os.path.dirname(  # stat_tag_matrix_plot
          os.path.abspath(sys.argv[0])))),
    'pipeline', 'src'))  # to import lib_stat_graph
import lib_stat_graph as lsg


class Strain(object):
  """ represents a strain genome, one column in the plot
  """
  def __init__(self, name, xml_0, xml_1=None, mode=None):
    self.name = name
    self.xml_0 = xml_0
    self.xml_1 = xml_1
    self.mode = mode
    self.graph_0 = lsg.readStatGraph(xml_0)
    if self.xml_1 is not None:
      self.graph_1 = lsg.readStatGraph(xml_1)
    self.transcript_level_tags = ['hasOkCopies', 'hasBadCopies',
                                  'ok', 'not_ok']
  def getRootValue_0(self):
    """ Report back the transcript value for the root tag.
    """
    t = lsg.getTagStats(self.graph_0, 'ok')  # can be any top level tag
    return t.nodeTranscripts  # not returing 'ok' but 'stats' here
  def getRootValue_1(self):
    """ Report back the transcript value for the root tag.
    """
    t = lsg.getTagStats(self.graph_1, 'ok')  # can be any top level tag
    return t.nodeTranscripts  # not returing 'ok' but 'stats' here
  def getValue_0(self, t):
    """ given a tag T, return the value from graph 0
    """
    return self.getValue(t, self.graph_0)
  def getValue_1(self, t):
    """ given a tag T, return the value from graph 0
    """
    return self.getValue(t, self.graph_1)
  def getValue(self, t, graph):
    s = lsg.getTagStats(graph, t)
    while s.children != []:
      assert len(s.children) == 1
      s = s.children[0]
    if t in self.transcript_level_tags:
      return s.tagTranscripts
    else:
      return s.tagTranscriptAnnotations
  def getDelta(self, t):
    """ given a tag T, return the delta between the two graphs
    """
    s0 = lsg.getTagStats(self.graph_0, t)
    s1 = lsg.getTagStats(self.graph_1, t)
    # Find the leaf StatCount node for both graphs
    while s0.children != []:
      assert len(s0.children) == 1
      s0 = s0.children[0]
    while s1.children != []:
      assert len(s1.children) == 1
      s1 = s1.children[0]
    if t in self.transcript_level_tags:
      t0 = s0.tagTranscripts
      t1 = s1.tagTranscripts
    else:
      t0 = s0.tagTranscriptAnnotations
      t1 = s1.tagTranscriptAnnotations
    if self.mode == 'delta':
      return t1 - t0
    elif self.mode == 'delta_percent':
      return 100. * (t1 - t0) / float(t0)


def InitializeArguments(parser):
  parser.add_argument('releases', nargs='+', type=str,
                      help='assembly release directories.')
  parser.add_argument('--mode', choices=['delta', 'raw', 'delta_percent'],
                      help='mode to plot. choices are delta or raw.')
  parser.add_argument('--exclude', type=str,
                      help='specify species to exclude, comma separated list.')
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
                      type=str, choices=['pdf', 'png', 'eps', 'all'],
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
  if args.mode.startswith('delta'):
    args.rows = ['stats', 'ok', 'ok:hasOkCopies', 'ok:hasBadCopies',
                 'not_ok', 'not_ok:hasOkCopies', 'not_ok:hasBadCopies',
                 'not_ok:noStop', 'not_ok:noStop:alignmentPartialMap',
                 'not_ok:noStop:alignmentPartialMap:alignmentAbutsEdge',
                 'not_ok:noStart', 'not_ok:noStart:alignmentPartialMap',
                 'not_ok:noStart:alignmentPartialMap:alignmentAbutsEdge',
                 'not_ok:nonsense',
                 'not_ok:synon', 'not_ok:nonsynon', 'not_ok:outOfFrame']
  else:
    args.rows = ['stats', 'ok', 'ok:hasOkCopies', 'ok:hasBadCopies',
                 'not_ok', 'not_ok:hasOkCopies', 'not_ok:hasBadCopies',
                 'not_ok:noStop', 'not_ok:noStop:alignmentPartialMap',
                 'not_ok:noStop:alignmentPartialMap:alignmentAbutsEdge',
                 'not_ok:noStart', 'not_ok:noStart:alignmentPartialMap',
                 'not_ok:noStart:alignmentPartialMap:alignmentAbutsEdge',
                 'not_ok:nonsense',
                 'not_ok:synon', 'not_ok:nonsynon', 'not_ok:outOfFrame']
  if args.releases is None:
    parser.error('Specify *sizes files!')
  if args.mode.startswith('delta') and len(args.releases) != 2:
    parser.error('not enough release directories specified for --mode delta.')
  if args.mode == 'raw' and len(args.releases) != 1:
    parser.error('too many release directories specified for --mode raw.')
  for d in args.releases:
    if not os.path.exists(d):
      parser.error('release directory %s does not exist.' % d)
    if not os.path.isdir(d):
      parser.error('%s is not a directory.' % d)
  if args.exclude is not None:
    args.exclude = args.exclude.split(',')
  else:
    args.exclude = []


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
    axDict: a dictionary of matplotlib axis objects
  Raises:
    ValueError: If an unknown spine location is passed.
  """
  # left 0.99 inches, right 0.25 inches, width 7.47 inches
  # bottom 0.34 inches, top 0.28 inches, height 3.04 inches
  args.axLeft = 0.99 / args.width
  args.axRight = 1.0 - (0.25 / args.width)
  args.axWidth = args.axRight - args.axLeft
  args.axBottom = 0.34 / args.height
  args.axTop = 1.0 - (0.28 / args.height)
  args.axHeight = args.axTop - args.axBottom
  xmargin = 0.01
  ymargin = 0.03
  ncol = len(data_list)
  nrow = len(args.rows)
  args.itemWidth = (args.axWidth - (ncol - 1) * xmargin) / float(ncol)
  args.itemHeight = (args.axHeight - (nrow - 1) * ymargin) / float(nrow)
  x, y = args.axLeft, args.axTop - args.itemHeight
  axDict = {}
  for r in args.rows:
    x = args.axLeft
    for i, c in enumerate(data_list):
      ax = fig.add_axes([x, y,
                         args.itemWidth, args.itemHeight])
      FixAxis(i, ax)
      axDict[(r, c.name)] = ax
      x += xmargin + args.itemWidth
    y -= ymargin + args.itemHeight
  return axDict


def FixAxis(i, ax):
  """ if this is the first column, print the y-axis, else do not
  """
  ax.yaxis.set_major_locator(pylab.NullLocator())
  ax.xaxis.set_major_locator(pylab.NullLocator())
  for loc, spine in ax.spines.iteritems():
    if not i and loc in ['left',]:
      spine.set_position(('outward', 10))
    elif i and loc in ['left',]:
      spine.set_color('none')
    elif loc in ['right', 'bottom', 'top']:
      spine.set_color('none')
    else:
      raise ValueError('unknown spine location: %s' % loc)
  ax.xaxis.set_ticks_position('bottom')
  if not i:
    ax.yaxis.set_ticks_position('left')
  # ax.axis('off')  # debugging layout
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
  """ read all the things.
  """
  if args.mode.startswith('delta'):
    return ReadFilesDelta(args)
  else:
    return ReadFilesRaw(args)


def ReadFilesRaw(args):
  """ read all the things and then return a list of Strain objects.
  """
  data = []
  for r in args.releases:
    xmls = glob(os.path.join(r, '*', 'stats.xml'))
    for x in xmls:
      # get all xmls
      name = os.path.split(os.path.dirname(x))[1].split('.')[1]
      if name in args.exclude:
        continue
      data.append(Strain(name, x))
  data = sorted(data, key=lambda d: d.getValue_0('ok'), reverse=True)
  return data


def ReadFilesDelta(args):
  """ read all the things and then return a list of Strain objects.
  """
  pairs = {} # keyed by strain name, valued by list (paths to xmls)
  for r in args.releases:
    xmls = glob(os.path.join(r, '*', 'stats.xml'))
    for x in xmls:
      # get all xmls
      name = os.path.split(os.path.dirname(x))[1].split('.')[1]
      if name in args.exclude:
        continue
      strain = pairs.setdefault(name, [])
      strain.append(x)
  data = []
  # only read strains that have two xmls
  for name, value in pairs.items():
    if len(value) == 2:
      data.append(Strain(name, value[0], value[1], args.mode))
  data = sorted(data, key=lambda d: d.getValue_1('ok'), reverse=True)
  return data


def GetLimits(r, data_list, args):
  """ Given a row name and the data_list return the x and y limits.
  """
  xlim = [0, 1]  # this is invariant in this application
  ylim = [sys.maxint, -sys.maxint]
  for d in data_list:
    if r == 'stats':
      v0 = d.getRootValue_0()
      if args.mode.startswith('delta'):
        v1 = d.getRootValue_1()
        ylim[0] = min(ylim[0], min(v0, v1))
        ylim[1] = max(ylim[1], max(v0, v1))
      else:
        ylim[0] = min(ylim[0], v0)
        ylim[1] = max(ylim[1], v0)
    else:
      if args.mode.startswith('delta'):
        v = d.getDelta(r)
      else:
        v = d.getValue_0(r)
      ylim[0] = min(ylim[0], v)
      ylim[1] = max(ylim[1], v)
  # ensure that 0 is present in the yaxis
  ylim[0] = min(ylim[0], 0)
  ylim[1] = max(ylim[1], 0)
  if ylim[0] == sys.maxint:
    print 'weird row: %s' % r
  return xlim, ylim


def PlotData(data_list, axDict, args):
  i = 0
  drawTitle = True
  for r in args.rows:
    drawYAxis = True
    xlim, ylim = GetLimits(r, data_list, args)
    for d in data_list:
      PlotOne(r, d, axDict[(r, d.name)], args, i, drawYAxis,
              xlim, ylim, drawTitle)
      i += 1
      drawYAxis = False
    drawTitle = False


def PlotOne(r, d, ax, args, j, drawYAxis, xlim, ylim, drawTitle):
  """ Plot one axes worth of data. d is a Strain object
  """
  if args.mode.startswith('delta'):
    PlotOneDelta(r, d, ax, args, j, drawYAxis, xlim, ylim, drawTitle)
  else:
    PlotOneRaw(r, d, ax, args, j, drawYAxis, xlim, ylim, drawTitle)


def PlotOneDelta(r, d, ax, args, j, drawYAxis, xlim, ylim, drawTitle):
  if r == 'stats':
    v0 = d.getRootValue_0()
    v1 = d.getRootValue_1()
    v = 1
  else:
    v = d.getDelta(r)
  if v == 0 or v is None:
    v = -0.1  # small negative nudge
  color = GetColor(v)
  ax.add_line(lines.Line2D(xdata=[0.1, 0.9],
                           ydata=[0, 0],
                           color='gray',
                           linewidth=0.5))
  if v > 0:
    va = 'bottom'
  else:
    va = 'top'
  if r != 'stats':
    ax.add_patch(patches.Rectangle((0.33, 0), 0.33, v, facecolor=color,
                                   linewidth=None,
                                   edgecolor='none',))
    ax.text(0.5, v, '%d' % v, ha='center', va=va, size='x-small')
  else:
    ax.add_patch(patches.Rectangle((0.25, 0), 0.25, v0, facecolor=color,
                                   linewidth=None,
                                   edgecolor='none',))
    ax.add_patch(patches.Rectangle((0.5, 0), 0.25, v1, facecolor=color,
                                   linewidth=None,
                                   edgecolor='none',))
    ax.text(0.33, v0, '%d' % v0, ha='center', va=va, size='xx-small')
    ax.text(0.66, v1, '%d' % v1, ha='center', va=va, size='xx-small')
  ax.set_xlim(xlim)
  ax.set_ylim(ylim)
  if drawTitle:
    ax.set_title(d.name)
  ax.set_xticks([])
  if drawYAxis:
    ax.locator_params(axis='y', nbins=3)
    ax.text(0.5, 0, r, ha='center', va='bottom', size='xx-small')
    ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 999))
  else:
    ax.set_yticks([])


def PlotOneRaw(r, d, ax, args, j, drawYAxis, xlim, ylim, drawTitle):
  if r == 'stats':
    v = d.getRootValue_0()
  else:
    v = d.getValue_0(r)
  if v == 0 or v is None:
    v = -0.1  # small negative nudge
  color = GetColor(v)
  ax.add_line(lines.Line2D(xdata=[0.1, 0.9],
                           ydata=[0, 0],
                           color='gray',
                           linewidth=0.5))
  if v > 0:
    va = 'bottom'
  else:
    va = 'top'
  if r != 'stats':
    ax.add_patch(patches.Rectangle((0.33, 0), 0.33, v, facecolor=color,
                                   linewidth=None,
                                   edgecolor='none',))
  else:
    ax.add_patch(patches.Rectangle((0.33, 0), 0.33, v, facecolor=color,
                                   linewidth=None,
                                   edgecolor='none',))
  ax.text(0.5, v, '%d' % v, ha='center', va=va, size='xx-small')
  ax.set_xlim(xlim)
  ax.set_ylim(ylim)
  if drawTitle:
    ax.set_title(d.name)
  ax.set_xticks([])
  if drawYAxis:
    ax.locator_params(axis='y', nbins=3)
    ax.text(0.5, 0, r, ha='center', va='bottom', size='xx-small')
    ax.ticklabel_format(style='sci', axis='y', scilimits=(0, 999))
  else:
    ax.set_yticks([])


def GetColor(n):
  """ Given an integer n, return a color to use for plotting.
  """
  if n > 0:
    return (98 / 255.0, 162 / 255.0, 209 / 255.0)
  else:
    return (233 / 255.0, 133 / 255.0, 131 / 255.0)


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
