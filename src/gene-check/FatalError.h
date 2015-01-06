#ifndef UTILS_FATALERROR_H
#define UTILS_FATALERROR_H

#include <typeinfo>
#include <stdexcept>
#include <string>
#include <iostream>
#include "Exception.h"

/**
 * Fatal error.  A generic error indicate that an operation failed
 * and things are not recoverable.
 *
 * @author Mark Diekhans &lt;markd@cse.ucsc.edu&gt;
 * @see G-KnownUtils::fatal
 */
class FatalError: public Exception {
public:
    /**
     * Construct a new exception.
     *
     * @param errDesc A description of the error.
     */
    FatalError(const string& errDesc): Exception(errDesc) {
    }

    /**
     * Destructor.
     */
    virtual ~FatalError() throw() {
    }
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

