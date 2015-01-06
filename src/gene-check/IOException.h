#ifndef IOEXCEPTION_H
#define IOEXCEPTION_H

#include <typeinfo>
#include "Exception.h"
#include "Format.h"
#include <string.h>

/**
 * I/O exception. 
 */
class IOException: public Exception {
  private:
  public:
    /**
     * Construct a new exception.
     *
     * @param errDesc A description of the error.
     */
    IOException(const string& errDesc):
        Exception(errDesc) {
    }

    /**
     * Construct a new exception.
     *
     * @param errDesc A description of the error.
     * @param fileName The file the error occured in.
     * @param lineNum The line number the error occured at (optional)
     */
    IOException(const string& errDesc, 
                const string& fileName,
                int lineNum = -1):
        Exception(errDesc + ": " + fileName
                  + ((lineNum >= 0)
                   ? (": " + Format::format("%d", lineNum)) : "")) {
    }

    /**
     * Construct a new exception.
     *
     * @param errNo Unix error number.
     * @param errDesc A description of the error.
     */
    IOException(int errNo,
                const string& errDesc):
        Exception(errDesc + ": " + strerror(errNo)) {
    }

    /**
     * Construct a new exception.
     *
     * @param errNo Unix error number.
     * @param errDesc A description of the error.
     * @param fileName The file the error occured in.
     * @param lineNum The line number the error occured at (optional)
     */
    IOException(int errNo,
                const string& errDesc, 
                const string& fileName,
                int lineNum = -1):
        Exception(errDesc + ": " + strerror(errNo) + ": \"" + fileName
                  + "\"" + ((lineNum >= 0) ?
                            (": " + Format::format("%d", lineNum)) : "")) {
    }

    /**
     * Destructor.
     */
    virtual ~IOException() throw() {
    }
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

