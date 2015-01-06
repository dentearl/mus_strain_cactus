
#include "FileOps.h"
#include "IOException.h"
#include "FIOStream.h"
#include "SysInfo.h"
#include "Convert.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * Does a file exist?
 */
bool FileOps::exists(const string& path) {
    struct stat statbuf;
    return (stat(path.c_str(), &statbuf) == 0);
}

/**
 * Make a directory, but don't fail if it already exists.
 */
void FileOps::makeDir(const char* path) {
    if (mkdir(path, 0777) < 0) {
        if ((errno != EEXIST)) {
            throw IOException(errno, "can't create directory", path);
        }
    }
}

/**
 * Create a heirarchy of directories.
 */
void FileOps::makeDir(const string& path) {
    char* slash;
    char hold;
    char pathBuf[path.size()+1];
    strcpy(pathBuf, path.c_str());

    // Create intermediate directories
    char* scanp = pathBuf;
    if (*scanp == '/') {
        scanp++;  // Skip initial directory
    }
    while ((slash = strchr(scanp, '/')) != NULL) {
        hold = *slash;
        *slash = '\0';
        makeDir(pathBuf);
        *slash = hold;
        scanp = slash+1;
    }
    // Last directory
    makeDir(pathBuf);
}

/**
 * Get the tmp directory, check for the TMPDIR env variable.
 */
string FileOps::getTmpDir() {
    const char* tmpDir = getenv("TMPDIR");
    if (tmpDir == NULL) {
        tmpDir = "/var/tmp";
    }
    return string(tmpDir);
}

/**
 * Create a temporary file in default tmpdir
 */
string FileOps::makeTmpFile(const string& baseName,
                            const string& ext) {
    return makeTmpFile(getTmpDir(), baseName, ext);
}

/**
 * Create a temporary file in a specified tmpdir
 */
string FileOps::makeTmpFile(const string& tmpDir,
                            const string& baseName,
                            const string& ext) {
    static const int MAX_TRIES = 512;
    string prefix = tmpDir + "/" + baseName + "."
        + SysInfo::getHostName() + "." + Convert::toString(getpid()) + ".";

    for (int cnt = 0; cnt < MAX_TRIES; cnt++) {
        string tmpFile = prefix + Convert::toString(cnt) + "." + ext;
        if (!exists(tmpFile)) {
            return tmpFile;
        }
    }

    // looped too many time (will probably never happen
    throw IOException("Can't create tmp file: too many file exist with "
                      "names in the form: " + prefix + "*." + ext);
}

/** 
 * chmod a file.
 */
void FileOps::chmod(const string& fname,
                    int mode) {
    if (::chmod(fname.c_str(), mode) != 0) {
        throw IOException(errno, "chmod failed", fname);
    }
}

/** 
 * Rename a file.
 */
void FileOps::rename(const string& oldName,
                     const string& newName) {
    if (::rename(oldName.c_str(), newName.c_str()) != 0) {
        throw IOException(errno, "rename of \"" + oldName
                          + "\" to \"" + newName + "\" failed");
    }
}

/**
 * Create  directories for a file, if they don't exist.
 */
void FileOps::makeFileDirs(const string& filePath) {
    string dirPath = dir(filePath);
    if (dirPath != ".") {
        makeDir(dirPath);
    }
}

/**
 * Extract the directory name of a file (or . in no directory).
 */
string FileOps::dir(const string& path) {
    size_t idx = path.rfind('/');
    if (idx == string::npos) {
        return string(".");
    } else {
        return path.substr(0, idx);
    }
}

/**
 * Extract the last component of a file path.
 */
string FileOps::tail(const string& path) {
    size_t idx = path.rfind('/');
    if ((idx == path.size()-1) && (path.size() > 1)) {
        // handle trainling slash.
        idx = path.rfind('/', idx-1);
    }
    if (idx == string::npos) {
        return path;
    } else {
        return path.substr(idx+1);
    }
}

/**
 * Get the name of the file, excluding it's extension.
 */
string FileOps::root(const string& path) {
    // Find last dot in last file name component
    size_t dotIdx = path.find_last_of(".");
    if ((dotIdx == string::npos)
        || (path.find_first_of("/", dotIdx) != string::npos)) {
        // no extension
        return path;
    } else {
        return path.substr(0, dotIdx);
    }
}

/**
 * Get the file extension
 */
string FileOps::ext(const string& path) {
    // Find last dot in last file name component
    size_t dotIdx = path.find_last_of(".");
    if ((dotIdx == string::npos)
        || (path.find_first_of("/", dotIdx) != string::npos)) {
        // no extension
        return "";
    } else {
        return path.substr(dotIdx+1);
    }
}

/**
 * Copy a file, will decompress input if compressed or
 * compress output if name ends in .gz.
 */
void FileOps::copy(const string& inName,
                   const string& outName) {
    const static int COPY_BUF_SIZE = 4*1024;
    char buf[COPY_BUF_SIZE];

    FIOStream in(inName, ios::in);
    FIOStream out(outName, ios::out);
    
    while (!(in.eof() || in.fail())) {
        in.read(buf, COPY_BUF_SIZE);
        int cnt = in.gcount();
        if (cnt > 0) {
            out.write(buf, cnt);
        }
    }
}


/**
 * Construct a file path from a file path and a directory that the directory
 * that the path is relative to.  If the path is absolute, it is returned
 * unchanged.
 */
string FileOps::relativePath(const string& relDir,
                             const string& filePath) {
    if ((filePath.size() == 0) || (filePath[0] == '/')
        || (relDir.size() == 0)) {
        // absolute file name, also just pass back empty string or
        // leave unchanged for empty directory.
        return filePath;
    } else {
        return relDir + "/" + filePath;
    }
}
