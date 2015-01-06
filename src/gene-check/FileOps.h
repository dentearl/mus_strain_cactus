#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <string>
using namespace std;

/**
 * Various operations on files.
 */
class FileOps {
  private:
    /**
     * Make a directory, but don't fail if it already exists.
     */
    static void makeDir(const char* path);

  public:
    /**
     * Does a file exist?
     */
    static bool exists(const string& path);

    /**
     * Create a heirarchy of directories.
     */
    static void makeDir(const string& path);

    /**
     * Get the tmp directory, check for the TMPDIR env variable.
     */
    static string getTmpDir();

    /**
     * Create a temporary file in default tmpdir
     */
    static string makeTmpFile(const string& baseName,
                              const string& ext);

    /**
     * Create a temporary file in a specified tmpdir
     */
    static string makeTmpFile(const string& tmpDir,
                              const string& baseName,
                              const string& ext);

    /** chmod a file. */
    static void chmod(const string& fname,
                      int mode);

    /** Rename a file. */
    static void rename(const string& oldName,
                       const string& newName);

    /**
     * Create  directories for a file, if they don't exist.
     */
    static void makeFileDirs(const string& filePath);

    /**
     * Extract the directory name of a file (or . in no directory).
     */
    static string dir(const string& path);

    /**
     * Extract the last component of a file path.
     */
    static string tail(const string& path);

    /**
     * Get the name of the file, excluding it's extension.
     */
    static string root(const string& path);

    /**
     * Get the file extension
     */
    static string ext(const string& path);

    /**
     * Copy a file, will decompress input if compressed or
     * compress output if name ends in .gz.
     */
    static void copy(const string& inName,
                     const string& outName);

    /**
     * Construct a file path from a file path and a directory that the
     * directory that the path is relative to.  If the path is absolute,
     * it is returned unchanged.
     */
    static string relativePath(const string& relDir,
                               const string& filePath);
};
#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
