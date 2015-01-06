import os
import subprocess


def RunCommands(cmds, local_temp_dir, in_pipes=None, out_pipes=None,
                 err_pipes=None, ignore_returns=False, serial=False):
  """ Wrapper for RunCommandsS and RunCommandsP.
  """
  if serial:
    RunCommandsSerial(cmds, local_temp_dir, in_pipes, out_pipes, err_pipes,
                      ignore_returns)
  else:
    RunCommandsParallel(cmds, local_temp_dir, in_pipes, out_pipes, err_pipes,
                        ignore_returns)


def HandlePipes(n, in_pipes, out_pipes, err_pipes, ignore_returns):
  """ Generate the correct data structures for RunCommands*() functions.
  """
  if in_pipes is None:
    in_pipes = [None] * n
  if out_pipes is None:
    out_pipes = [None] * n
  if err_pipes is None:
    err_pipes = [None] * n
  if ignore_returns is None or ignore_returns == False:
    ignore_returns = [False] * n
  return in_pipes, out_pipes, err_pipes, ignore_returns


def HandlePipesInstance(in_pipe, out_pipe, err_pipe):
  """ Generate the correct data structures for a single command.
  """
  if in_pipe is None:
    sin = None
  else:
    sin = subprocess.PIPE
  if out_pipe is None:
    sout = None
  else:
    sout = subprocess.PIPE
  if err_pipe is None:
    serr = None
  else:
    serr = subprocess.PIPE
  return sin, sout, serr


def RunCommandsSerial(cmds, local_temp_dir, in_pipes=None, out_pipes=None,
                      err_pipes=None, ignore_returns=None):
  """ Uses the subprocess module to issue serial processes from the cmds list.
  Arguments:
    ignore_returns: if true, return code is ignored
  """
  if not os.path.exists(local_temp_dir):
    raise ValueError('local_temp_dir "%s" does not exist.' % local_temp_dir)
  in_pipes, out_pipes, err_pipes, ignore_returns = HandlePipes(
    len(cmds), in_pipes, out_pipes, err_pipes, ignore_returns)
  for i, c in enumerate(cmds, 0):
    sin, sout, serr = HandlePipesInstance(in_pipes[i], out_pipes[i],
                                          err_pipes[i])
    p = subprocess.Popen(c, cwd=local_temp_dir, stdin=sin,
                         stdout=sout, stderr=serr)
    if in_pipes[i] is None:
      sin = None
    else:
      if not os.path.exists(in_pipes[i]):
        raise IOError('Unable to locate inPipe file: %s for command %s'
                      % (in_pipes[i], ' '.join(c)))
      sin = open(in_pipes[i], 'r').read()
    p_out, p_err = p.communicate(sin)
    if out_pipes[i] is None:
      if not ignore_returns[i]:
        HandleReturnCode(p.returncode, cmds[i])
    else:
      f = open(out_pipes[i], 'w')
      f.write(p_out)
      f.close()
    if err_pipes[i] is None:
      if not ignore_returns[i]:
        HandleReturnCode(p.returncode, cmds[i])
    else:
      g = open(err_pipes[i], 'w')
      g.write(p_err)
      g.close()
      if not ignore_returns[i]:
        HandleReturnCode(p.returncode, cmds[i])


def RunCommandsPipes(cmds, local_temp_dir, in_pipe=None, out_pipe=None,
                     err_pipe=None, ignore_returns=None):
  """ Uses the subprocess module to issue piped processes from the cmds list.
  Arguments:
    ignore_returns: if true, return code is ignored
  """
  if not os.path.exists(local_temp_dir):
    raise ValueError('local_temp_dir "%s" does not exist.' % local_temp_dir)
  sin, sout, serr = HandlePipesInstance(in_pipe, out_pipe, err_pipe)
  p_prev = None
  for i, c in enumerate(cmds, 0):
    if p_prev is None:
      p = subprocess.Popen(c, cwd=local_temp_dir, stdin=sin,
                           stdout=subprocess.PIPE, stderr=serr)
    else:
      p = subprocess.Popen(c, cwd=local_temp_dir, stdin=p_prev.stdout,
                           stdout=subprocess.PIPE, stderr=serr)
      p_prev.stdout.close()
    p_prev = p
  p_out, p_err = p.communicate()
  if out_pipe is None:
    if not ignore_returns:
      HandleReturnCode(p.returncode, cmds[i])
  else:
    f = open(out_pipe, 'w')
    f.write(p_out)
    f.close()
  if err_pipe is None:
    if not ignore_returns:
      HandleReturnCode(p.returncode, cmds[i])
  else:
    g = open(err_pipe, 'w')
    g.write(p_err)
    g.close()
    if not ignore_returns:
      HandleReturnCode(p.returncode, cmds[i])


def RunCommandsParallel(cmds, local_temp_dir, in_pipes=None, out_pipes=None,
                        err_pipes=None, ignore_returns=None, **kwargs):
  """ Uses the subprocess module to issue parallel processes from cmds list.
  """
  if not os.path.exists(local_temp_dir):
    raise ValueError('local_temp_dir "%s" does not exist.' % local_temp_dir)
  procs = []
  in_pipes, out_pipes, err_pipes, ignore_returns = HandlePipes(
    len(cmds), in_pipes, out_pipes, err_pipes, ignore_returns)
  for i, c in enumerate(cmds, 0):
    sin, sout, serr = HandlePipesInstance(in_pipes[i], out_pipes[i],
                                          err_pipes[i])
    procs.append(subprocess.Popen(
        c, cwd=local_temp_dir, stdin=sin, stdout=sout, stderr=serr))
  for i, p in enumerate(procs, 0):
    if in_pipes[i] is None:
      sin = None
    else:
      if not os.path.exists(in_pipes[i]):
        raise IOError('Unable to locate inPipe file: %s for command %s'
                      % (in_pipes[i], cmds[i]))
      sin = open(in_pipes[i], 'r').read()
    p_out, p_err = p.communicate(sin)
    if out_pipes[i] is None:
      if not ignore_returns[i]:
        HandleReturnCode(p.returncode, cmds[i])
    else:
      f = open(out_pipes[i], 'w')
      f.write(p_out)
      f.close()
    if err_pipes[i] is None:
      if not ignore_returns[i]:
        HandleReturnCode(p.returncode, cmds[i])
    else:
      g = open(err_pipes[i], 'w')
      g.write(p_err)
      g.close()
      if not ignore_returns[i]:
        HandleReturnCode(p.returncode, cmds[i] + ['< %s 1> %s 2> %s'
                                                  % (in_pipes[i],
                                                     out_pipes[i],
                                                     err_pipes[i])])


def HandleReturnCode(retcode, cmd):
  """ Handle the return codes from RunCommands.
  """
  if not isinstance(retcode, int):
    raise TypeError('HandleReturnCode takes an integer for '
                    'retcode, not a %s.' % retcode.__class__)
  if not isinstance(cmd, list):
    raise TypeError('HandleReturnCode takes a list for '
                    'cmd, not a %s.' % cmd.__class__)
  if retcode:
    if retcode < 0:
      raise RuntimeError('Experienced an error while trying to execute: '
                         '%s SIGNAL:%d' %(' '.join(cmd), -retcode))
    else:
      raise RuntimeError('Experienced an error while trying to execute: '
                         '%s retcode:%d' %(' '.join(cmd), retcode))


def Which(program, extra_path_list=None):
  """ Which() acts like the unix utility which, but is portable between os.
  If the program does not exist in the PATH then 'None' is returned.
  extra_path_list is an os-appropritate list of paths to start the search
  for the executable.
  """
  if extra_path_list is not None:
    if not isinstance(extra_path_list, list):
      raise RuntimeError('extra_path_list must be a list, not a %s'
                         % extra_path_list.__class__)
  def is_exe(fpath):
    return os.path.exists(fpath) and os.access(fpath, os.X_OK)
  fpath, fname = os.path.split(program)
  if fpath != '':
    if is_exe(program):
      return program
  else:
    if extra_path_list is not None:
      for path in extra_path_list:
        exe_file = os.path.join(path, program)
        if is_exe(exe_file):
          return exe_file
    for path in os.environ["PATH"].split(os.pathsep):
      exe_file = os.path.join(path, program)
      if is_exe(exe_file):
        return exe_file
    exe_file = os.path.join(os.getcwd(), program)
    if is_exe(exe_file):
      return exe_file
  return None


def Touch(path):
  """ Open PATH and touch the file without modifying, create if non-existant.
  """
  with open(path, 'a'):
    os.utime(path, None)


def PrettyTime(t):
  """ Given input t as seconds, return a nicely formatted string.
  """
  from math import floor
  plural_dict = {True: 's', False: ''}
  if t < 120:
    return '%ds' % t
  if t < 120 * 60:
    m = floor(t / 60.)
    s = t % 60
    return '%dm %ds' % (m, s)
  if t < 25 * 60 * 60:
    h = floor(t / 60. / 60.)
    m = floor((t - (h * 60. * 60.)) / 60.)
    s = t % 60
    return '%dh %gm %ds' % (h, m, s)
  if t < 7 * 24 * 60 * 60:
    d = floor(t / 24. / 60. / 60.)
    h = floor((t - (d * 24. * 60. * 60.)) / 60. / 60.)
    m = floor((t
               - (d * 24. * 60. * 60.)
               - (h * 60. * 60.))
              / 60.)
    s = t % 60
    d_plural = plural_dict[d > 1]
    return '%d day%s %dh %dm %ds' % (d, d_plural, h, m, s)
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
  return '%d week%s %d day%s %dh %dm %ds' % (w, w_plural, d,
                                             d_plural, h, m, s)


def PrettyTimeToSeconds(t):
  """ Given a string from the PrettyTime() function, turn it back into seconds.
  """
  def triageWeek(t):
    assert(len(t) == 7)
    assert(t[1].startswith('week'))
    s = 7 * 24 * 60 * 60 * int(t[0])
    return s + triageDay(t[2:])
  def triageDay(t):
    assert(len(t) == 5)
    assert(t[1].startswith('day'))
    s = 24 * 60 * 60 * int(t[0])
    return s + triageHour(t[2:])
  def triageHour(t):
    assert(len(t) == 3)
    assert(t[0].endswith('h'))
    s = 60 * 60 * int(t[0][:-1])
    return s + triageMinute(t[1:])
  def triageMinute(t):
    assert(len(t) == 2)
    assert(t[0].endswith('m'))
    s = 60 * int(t[0][:-1])
    return s + triageSecond(t[1:])
  def triageSecond(t):
    assert(len(t) == 1)
    assert(t[0].endswith('s'))
    s = int(t[0][:-1])
    return s
  d = t.split()
  # triage the input to determine the leading unit
  try:
    n = float(d[0])
    # either week or day data leads the list
    if d[0].endswith('week'):
      return triageWeek(d)
    else:
      return triageDay(d)
  except ValueError:
    # if there's a letter on the end then it's one of these three
    if d[0].endswith('h'):
      return triageHour(d)
    elif d[0].endswith('m'):
      return triageMinute(d)
    elif d[0].endswith('s'):
      return triageSecond(d)
    else:
      print ('wtf is wrong with this prettyString: '
             '->%s<-, d[0]=%s' % (t, d[0]))
      raise


def TimeStamp(out_path, time_start=None, name=None, tag=''):
  """ Open up the log file and make a timestamp.
  """
  import time
  now = time.time()
  tag_s = ''
  if tag != '':
    tag_s = '  # %s' % tag
  if name is None:
    filename = os.path.join(out_path, 'jt_issued_commands.log')
  else:
    filename = os.path.join(out_path, name)
  f = open(filename, 'a')
  if time_start is not None:
    elapsed_time = now - time_start
    f.write('[%s] End (elapsed: %s)%s\n' %
            (TimeString(now), PrettyTime(elapsed_time), tag_s))
  else:
    f.write('[%s] Start%s\n' % (TimeString(now), tag_s))
  f.close()
  return now


def TimeString(then=None):
  """ Return a prtty timestring for use with timestamps.
  """
  import time
  if then is None:
    return time.strftime("%a, %d %b %Y %H:%M:%S (%Z)",
                         time.localtime(time.time()))
  else:
    return time.strftime("%a, %d %b %Y %H:%M:%S (%Z)",
                         time.localtime(then))


def LogCommand(out_path, cmds, out_pipe=None, err_pipe=None, name=None):
  """ Write out the commands that will be executed for this run.
  """
  import time
  if name is None:
    filename = os.path.join(out_path, 'jt_issued_commands.log')
  else:
    filename = os.path.join(out_path, name)
  f = open(filename, 'a')
  out_str = ''
  if out_pipe is not None:
    out_str = ' 1>%s' % ' '.join(out_pipe)
  if err_pipe is None:
    err_str = ''
  else:
    err_str = ' 2>%s' % ' '.join(err_pipe)
  for c in cmds:
    f.write('[%s] %s%s%s\n' % (TimeString(),
                               ' '.join(c),
                               err_str, out_str))
  f.close()


def LogCommandPipe(out_path, cmds, out_pipe=None, err_pipe=None, name=None):
  """ Write out the pipe chained commands that will be executed for this run.
  """
  import time
  if name is None:
    filename = os.path.join(out_path, 'jt_issued_commands.log')
  else:
    filename = os.path.join(out_path, name)
  f = open(filename, 'a')
  log = ''
  log = ' | '.join(map(lambda c: ' '.join(c), cmds))
  if out_pipe is None:
    out_str = ''
  else:
    out_str = ' 1>%s' % ' '.join(out_pipe)
  if err_pipe is None:
    err_str = ''
  else:
    err_str = ' 2>%s' % ' '.join(err_pipe)
  for c in cmds:
    f.write('[%s] %s%s%s\n' % (TimeString(),
                               log,
                               err_str, out_str))
  f.close()
