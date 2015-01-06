"""
Functions to support transmap programs"
"""
import tempfile, subprocess, os, sys

class ProgRunner(object):
    "run a program with optional verbose tracing"
    def __init__(self, verbose):
        self.verbose = verbose

    def __trace(self, cmd):
        if self.verbose:
            sys.stderr.write(" ".join(cmd) + "\n")
    def run(self, cmd):
        self.__trace(cmd)
        subprocess.check_call(cmd)

    def __call__(self, cmd):
        self.run(cmd)

    def runOutput(self, cmd):
        self.__trace(cmd)
        return subprocess.check_output(cmd)

class IntermediateFiles(object):
    "create either temporary or saved intermediates file"
    def __init__(self, intermediateFilePrefix):
        "intermediateFilePrefix is None to create temporary files"
        self.intermediateFilePrefix = intermediateFilePrefix
        self.tmpFiles = []

    def mkfile(self, suffix):
        if self.intermediateFilePrefix != None:
            return self.intermediateFilePrefix + suffix
        else:
            fd, tmpPath = tempfile.mkstemp(suffix=suffix)
        os.close(fd)
        self.tmpFiles.append(tmpPath)
        return tmpPath

    def delete(self):
        "delete non-saved intermediate files"
        for tmpFile in self.tmpFiles:
            try:
                os.unlink(tmpFile)
            except:
                pass
        self.tmpFiles = []

    def __del__(self):
        "tmp files deleted on object deletion"
        self.delete()

