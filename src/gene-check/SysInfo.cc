
#include "SysInfo.h"
#include "FatalError.h"
#include "StringVector.h"
#include "Convert.h"
#include <unistd.h>
#include <fstream>

/** Cache of hostName */
string SysInfo::sHostName;

/**
 * Get the name of this host.
 */
const string& SysInfo::getHostName() {
    if (sHostName.size() == 0) {
        char buf[256];
        if (gethostname(buf, sizeof(buf)) < 0) {
            throw FatalError("gethostname failed");
        }
        sHostName = buf;
    }
    return sHostName;
}

#ifdef __linux__
/* 
 * On linux, return the process virtual memory, in floating point megabytes
 * Parse size out of /proc stat file.
 */
double SysInfo::getVMSize() {
    static const int VM_IDX = 21;
    double vsize = 0.0;
    ifstream in("/proc/self/stat");
    if (in.is_open()) {
        // vmsize is word 22, seperated by single blank
        string buf;
        getline(in, buf);
        StringVector words(StringVector::splitOnWhiteSpace(buf));
        vsize = Convert::toDouble(words[VM_IDX]);
    }
    return vsize/(1024.0*1024.0);
}
#else
/* 
 * Return 0.0 for current process virtual memory on OS were this isn't
 * implemented.
 */
double SysInfo::getVMSize() {
    return 0.0;
}
#endif
