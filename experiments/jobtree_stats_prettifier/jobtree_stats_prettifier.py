#!/usr/bin/env python
"""
takes a jobTreeStats xml output and formats it for a human to read.

22 may 2014
dent earl, dearl (a) soe ucsc edu
"""
from argparse import ArgumentParser
import xml.etree.ElementTree as ET

class JTTag(object):
  def __init__(self, tree):
    """ Given an ElementTree tag, build a convenience object.
    """
    for name in ['total_time', 'median_clock', 'total_memory', 'median_wait',
                'total_number', 'average_time', 'median_memory',
                'min_number_per_slave', 'average_wait', 'total_clock',
                'median_time', 'min_time', 'min_wait', 'max_clock',
                'max_wait', 'total_wait', 'min_clock', 'average_memory',
                'max_number_per_slave', 'max_memory', 'min_clock',
                'average_memory', 'max_number_per_slave', 'max_memory',
                'median_number_per_slave', 'average_number_per_slave',
                'max_time', 'average_clock', 'min_memory'
                ]:
      setattr(self, name, self.__get(tree, name))
      self.name = tree.tag
  def __get(self, tag, name):
    if name in tag.attrib:
      value = tag.attrib[name]
    else:
      return float('nan')
    try:
      a = float(value)
    except ValueError:
      a = float('nan')
    return a


def InitializeArguments(parser):
  parser.add_argument('xml', help='path to jobTreeStats output.')
  parser.add_argument('--pretty', action='store_true', default=False,
                      help='prettify the numbers to be human readable.')
  parser.add_argument('--categories',
                      help=('comma separated list from [time, clock, wait, '
                            'memory]'))
  parser.add_argument('--sortby', default='time',
                      help=('how to sort Target list. may be from [time, '
                            'clock, wait, memory, count]. default=%(default)s'))
  parser.add_argument('--reverse_sort', default=False, action='store_true',
                      help='reverse sort order.')


def CheckArguments(args, parser):
  if args.xml is None:
    parser.error('Specify the path to input xml.')
  default_categories = ['time', 'clock', 'wait', 'memory']
  if args.categories is None:
    args.categories = default_categories
  else:
    args.categories = args.categories.split(',')
  for c in args.categories:
    if c not in default_categories:
      parser.error('Unknown category %s. Must be from %s'
                   % (c, str(default_categories)))
  if args.sortby is not None:
    if args.sortby not in default_categories and args.sortby != 'count':
      parser.error('Unknown --sortby %s. Must be from %s'
                   % (args.sortby, str(default_categories)))


def PadStr(s, field=None):
  """ Pad the begining of a string with spaces, if necessary.
  """
  if field is None:
    return s
  else:
    if len(s) >= field:
      return s
    else:
      return ' ' * (field - len(s)) + s


def PrettyMemory(k, field=None, is_bytes=False):
  """ Given input k as kilobytes, return a nicely formatted string.
  """
  from math import floor
  if is_bytes:
    k /= 1024
  if k < 1024:
    return PadStr('%gK' % k, field)
  if k < (1024 * 1024):
    return PadStr('%.1fM' % (k / 1024.0), field)
  if k < (1024 * 1024 * 1024):
    return PadStr('%.1fG' % (k / 1024.0 / 1024.0), field)
  if k < (1024 * 1024 * 1024 * 1024):
    return PadStr('%.1fT' % (k / 1024.0 / 1024.0 / 1024.0), field)
  if k < (1024 * 1024 * 1024 * 1024 * 1024):
    return PadStr('%.1fP' % (k / 1024.0 / 1024.0 / 1024.0 / 1024.0), field)


def PrettyTime(t, field=None):
  """ Given input t as seconds, return a nicely formatted string.
  """
  from math import floor
  plural_dict = {True: 's', False: ''}
  if t < 120:
    return PadStr('%ds' % t, field)
  if t < 120 * 60:
    m = floor(t / 60.)
    s = t % 60
    return PadStr('%dm%ds' % (m, s), field)
  if t < 25 * 60 * 60:
    h = floor(t / 60. / 60.)
    m = floor((t - (h * 60. * 60.)) / 60.)
    s = t % 60
    return PadStr('%dh%gm%ds' % (h, m, s), field)
  if t < 7 * 24 * 60 * 60:
    d = floor(t / 24. / 60. / 60.)
    h = floor((t - (d * 24. * 60. * 60.)) / 60. / 60.)
    m = floor((t
               - (d * 24. * 60. * 60.)
               - (h * 60. * 60.))
              / 60.)
    s = t % 60
    d_plural = plural_dict[d > 1]
    return PadStr('%dday%s%dh%dm%ds' % (d, d_plural, h, m, s), field)
  w = floor(t / 7. / 24. / 60. / 60.)
  d = floor((t - (w * 7 * 24 * 60 * 60)) / 24. / 60. / 60.)
  h = floor((t
             - (w * 7. * 24. * 60. * 60.)
             - (d * 24. * 60. * 60.))
            / 60. / 60.)
  m = floor((t
             - (w * 7. * 24. * 60. * 60.)
             - (d * 24. * 60. * 60.)
             - (h * 60. * 60.))
            / 60.)
  s = t % 60
  w_plural = plural_dict[w > 1]
  d_plural = plural_dict[d > 1]
  return PadStr('%dweek%s%dday%s%dh%dm%ds' % (w, w_plural, d,
                                             d_plural, h, m, s), field)


def OpenXml(args):
  """ open args.xml and return a ready-to process ElementTree.
  """
  return ET.parse(args.xml)


def ReportTime(t, args, field=None):
  """ Given t seconds, report back the correct format as string.
  """
  if args.pretty:
    return PrettyTime(t, field=field)
  else:
    if field is not None:
      return '%*.2f' % (field, t)
    else:
      return '%.2f' % t


def ReportMemory(k, args, field=None, is_bytes=False):
  """ Given k kilobytes, report back the correct format as string.
  """
  if args.pretty:
    return PrettyMemory(k, field=field, is_bytes=is_bytes)
  else:
    if is_bytes:
      k /= 1024
    if field is not None:
      return '%*gK' % (field, k)
    else:
      return '%gK' % k


def ReportNumber(n, args, field=None):
  """ Given n an integer, report back the correct format as string.
  """
  if field is not None:
    return '%*g' % (field, n)
  else:
    return '%g' % n


def ProcessData(xml_tree, args):
  """ walk the xml_tree and gather up the important bits.
  """
  root = xml_tree.getroot()
  slave = JTTag(root.find('slave'))
  target = JTTag(root.find('target'))
  target_types_tree = root.find('target_types')
  target_types = []
  for child in target_types_tree:
    target_types.append(JTTag(child))
  return root, slave, target, target_types


def PrintTag(key, tag, args):
  """ Print out a JTTag()
  """
  header = '  %7s ' % 'Count'
  sub_header = '  %7s ' % 'n'
  tag_str = '  %s' % ReportNumber(tag.total_number, args, field=7)
  if key == 'target':
    print ' %-12s | %7s%7s%7s%7s ' % ('Slave Jobs', 'min', 'med', 'ave', 'max')
    slave_str = '%s| ' % (' ' * 14)
    for t in [tag.min_number_per_slave, tag.median_number_per_slave,
              tag.average_number_per_slave, tag.max_number_per_slave]:
      slave_str += ReportNumber(t, args, field=7)
    print slave_str
  if 'time' in args.categories:
    header += '| %40s ' % 'Time'
    sub_header += '| %10s%10s%10s%10s ' % ('min', 'med', 'ave', 'max')
    tag_str += ' | '
    for t in [tag.min_time, tag.median_time,
              tag.average_time, tag.max_time]:
      tag_str += ReportTime(t, args, field=10)
  if 'clock' in args.categories:
    header += '| %40s ' % 'Clock'
    sub_header += '| %10s%10s%10s%10s ' % ('min', 'med', 'ave', 'max')
    tag_str += ' | '
    for t in [tag.min_clock, tag.median_clock,
              tag.average_clock, tag.max_clock]:
      tag_str += ReportTime(t, args, field=10)
  if 'wait' in args.categories:
    header += '| %40s ' % 'Wait'
    sub_header += '| %10s%10s%10s%10s ' % ('min', 'med', 'ave', 'max')
    tag_str += ' | '
    for t in [tag.min_wait, tag.median_wait,
              tag.average_wait, tag.max_wait]:
      tag_str += ReportTime(t, args, field=10)
  if 'memory' in args.categories:
    header += '| %40s ' % 'Memory'
    sub_header += '| %10s%10s%10s%10s ' % ('min', 'med', 'ave', 'max')
    tag_str += ' | '
    for t in [tag.min_memory, tag.median_memory,
              tag.average_memory, tag.max_memory]:
      tag_str += ReportMemory(t, args, field=10)
  print header
  print sub_header
  print tag_str


def Get(tree, name):
  """ Return a float value attribute NAME from TREE.
  """
  if name in tree.attrib:
    value = tree.attrib[name]
  else:
    return float('nan')
  try:
    a = float(value)
  except ValueError:
    a = float('nan')
  return a


def SortTargets(target_types, args):
  """ Return a target_types all sorted.
  """
  if args.sortby == 'time':
    return sorted(target_types, key=lambda tag: tag.median_time,
                  reverse=args.reverse_sort)
  elif args.sortby == 'alpha':
    return sorted(target_types, key=lambda tag: tag.name,
                  reverse=args.reverse_sort)
  elif args.sortby == 'clock':
    return sorted(target_types, key=lambda tag: tag.median_clock,
                  reverse=args.reverse_sort)
  elif args.sortby == 'wait':
    return sorted(target_types, key=lambda tag: tag.median_wait,
                  reverse=args.reverse_sort)
  elif args.sortby == 'memory':
    return sorted(target_types, key=lambda tag: tag.median_memory,
                  reverse=args.reverse_sort)
  elif args.sortby == 'count':
    return sorted(target_types, key=lambda tag: tag.total_number,
                  reverse=args.reverse_sort)


def ReportData(root, slave, target, target_types, args):
  """ print the important bits out.
  """
  print 'Batch System: %s' % root.attrib['batch_system']
  s = ('Default CPU: %s  Default Memory: %s\n'
       'Job Time: %s  Max CPUs: %s  Max Threads: %s' % (
      ReportNumber(Get(root, 'default_cpu'), args),
      ReportMemory(Get(root, 'default_memory'), args, is_bytes=True),
      ReportTime(Get(root, 'job_time'), args),
      ReportNumber(Get(root, 'max_cpus'), args),
      ReportNumber(Get(root, 'max_threads'), args),
      ))
  print s
  s = ('Total Clock: %s  Total Runtime: %s' % (
      ReportTime(Get(root, 'total_clock'), args),
      ReportTime(Get(root, 'total_run_time'), args),
      ))
  print s
  print 'Slave'
  PrintTag('slave', slave, args)
  print 'Target'
  PrintTag('target', target, args)
  target_types = SortTargets(target_types, args)
  for t in target_types:
    print ' %s' % t.name
    PrintTag(t.name, t, args)


def main():
  parser = ArgumentParser()
  InitializeArguments(parser)
  args = parser.parse_args()
  CheckArguments(args, parser)
  xml_tree = OpenXml(args)
  root, slave, target, target_types = ProcessData(xml_tree, args)
  ReportData(root, slave, target, target_types, args)


if __name__ == '__main__':
  main()
