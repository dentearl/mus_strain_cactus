#!/usr/bin/env python
"""
newickToPhylodistance
19 August 2014
dent earl, dearl a soe ucsc edu

script to take a phylogenetic tree and output a list of
distances ordered by distance to a node in the tree.
"""
from argparse import ArgumentParser
from math import isnan
from bioio import newickTreeParser


class DistanceTree(object):
  def __init__(self):
    self.name = ''
    self.right = None
    self.left = None
    self.is_leaf = False
    self.tree_distance = float('nan')
    self.ref_distance = float('nan')


def InitializeArguments(parser):
  """
  """
  parser.add_argument(
    '--newick', type=str,
    default=('((SPRETEiJ:0.0020, (PWKPhJ:0.0010, (CASTEiJ:0.0010, '
             '(WSBEiJ:1.0E-5, (((NZOHlLtJ:1.0E-6, (C57B6NJ:1.0E-6, '
             'C57B6J:1.0E-6):1.0E-6):1.0E-6, ((NODShiLtJ:1.0E-6, '
             'FVBNJ:1.0E-6):1.0E-6, (((DBA2J:1.0E-6, (CBAJ:1.0E-6, '
             'C3HHeJ:1.0E-6):1.0E-6):1.0E-6, AKRJ:1.0E-6):1.0E-6, '
             '(BALBcJ:1.0E-6, AJ:1.0E-6):1.0E-6):1.0E-6):1.0E-6):1.0E-6, '
             '(LPJ:1.0E-6, 129S1:1.0E-6):1.0E-6):1.0E-6):1.0E-4):1.0E-6)'
             ':1.0E-6):0.013, Rattus:0.013);'))
  parser.add_argument('--reference', type=str, default='C57B6J')


def CheckArguments(args, parser):
  """
  """
  for n, v in [('newick', args.newick), ('reference', args.reference)]:
    if v is None:
      parser.error('Specify --%s' % n)


def BuildDistanceTree(nt, args):
  if nt is None:
    return None
  dt = DistanceTree()
  if nt.iD is None:
    nt.iD = 'Root'
  dt.name = nt.iD
  dt.tree_distance = nt.distance
  dt.left = BuildDistanceTree(nt.left, args)
  dt.right = BuildDistanceTree(nt.right, args)
  if dt.left is None and dt.right is None:
    dt.is_leaf = True
  return dt


def PushDistanceUp(dt, args):
  """
  descend the tree looking for the target species, if found
  return the cumulative distance up to the root.
  """
  if dt is None:
    return 0.0
  if dt.name == args.reference:
    dt.ref_distance = 0.0
    assert(dt.right is None)
    assert(dt.left is None)
    assert(dt.is_leaf is True)
    return dt.tree_distance
  r = PushDistanceUp(dt.right, args)
  l = PushDistanceUp(dt.left, args)
  assert(not(r > 0.0 and l > 0.0))
  if r > 0.0:
    dt.ref_distance = r
    return r + dt.tree_distance
  if l > 0.0:
    dt.ref_distance = l
    return l + dt.tree_distance
  return 0.0


def PushDistanceDown(dt, distance, args):
  """
  there is a path through the dt from the root down to
  the reference. Walk the tree and wherever there is no
  distance info, send the distance down the tree.
  """
  if dt is None:
    return
  if isnan(dt.ref_distance):
    dt.ref_distance = dt.tree_distance + distance
  PushDistanceDown(dt.left, dt.ref_distance, args)
  PushDistanceDown(dt.right, dt.ref_distance, args)


def RecordDistances(dt, args):
  PushDistanceUp(dt, args)
  PushDistanceDown(dt, 0., args)
  if dt is None:
    return
  RecordDistances(dt.left, args)
  RecordDistances(dt.right, args)


def ConvertTree(newick_tree, args):
  dt = BuildDistanceTree(newick_tree, args)
  RecordDistances(dt, args)
  return dt


def GetDistances(dt, args):
  if dt is None:
    return []
  if dt.is_leaf:
    return [(dt.name, dt.ref_distance)]
  distances = GetDistances(dt.left, args)
  distances += GetDistances(dt.right, args)
  return distances


def PrintDistances(distance_list, args):
  distances = sorted(distance_list, key=lambda x: x[1], reverse=False)
  for i in xrange(0, len(distances)):
    for j in xrange(i + 1, len(distances)):
      name_i, value_i = distances[i]
      name_j, value_j = distances[j]
      if name_i != args.reference and name_j != args.reference: continue
      print name_i, name_j, value_i + value_j


def main():
  parser = ArgumentParser()
  InitializeArguments(parser)
  args = parser.parse_args()
  CheckArguments(args, parser)
  tree = newickTreeParser(args.newick, 0.0)
  distance_tree = ConvertTree(tree, args)
  distances = GetDistances(distance_tree, args)
  PrintDistances(distances, args)


if __name__ == '__main__':
  main()
