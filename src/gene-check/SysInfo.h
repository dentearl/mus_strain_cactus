#ifndef SYSINFO_H
#define SYSINFO_H

#include <string>
using namespace std;

/**
 * Get various system information in a C++ friendly and system-indendent
 * way.
 */
class SysInfo {
  private:
    static string sHostName;
  public:
    /**
     * Get the name of this host.
     */
    static const string& getHostName();

    /* 
     * Return the current process virtual memory, in floating point megabytes, or
     * 0.0 if it can't be determined.
     */
    static double getVMSize();
};
#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
