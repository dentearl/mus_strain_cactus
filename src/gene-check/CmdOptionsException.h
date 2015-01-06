#ifndef CMD_OPTIONS_EXCEPTION_H
#define CMD_OPTIONS_EXCEPTION_H

#include <typeinfo>
#include <string>
#include "Exception.h"

/**
 * Exception for errors parsing a command line.
 */
class CmdOptionsException: public Exception {
public:
    /**
     * Construct a new exception.
     *
     * @param errDesc A description of the error.
     */
    CmdOptionsException(const string& errDesc): Exception(errDesc) {
    }
    
    /**
     * Destructor.
     */
    virtual ~CmdOptionsException() throw() {
    }
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

