#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>
#include <string>
#include <iostream>
#include <stdlib.h>
using namespace std;

/**
 * Base exception class for all G-Known exceptions.  This is especially
 * useful since we seem to have trouble getting exceptions to do anything
 * interesting, so we have it print out an error message and exit.
 * <p>
 * If the environment variable GK_ABORT is set, then we abort on an exception,
 * creating a core dump, otherwise exit.
 */
class Exception: public exception {
  private:
    string errorDesc;

  protected:
    /* 
     * Display exception and abort or exit, depending if GK_ABORT environment
     * variable is set.  Normally called by constructor unless noExit is
     * specified.
     */
    void displayAndExit();

  public:
    /**
     * Construct a new exception.
     *
     * @param errDesc A description of the error.
     * @param noExit don't call displayAndExit.
     */
    Exception(const string& errDesc,
              bool noExit = false);

    /** Destructor. */
    virtual ~Exception() throw() {
    }

    /**
     * Get the error description.
     */
    virtual const char* what() const throw() {
        return errorDesc.c_str();
    }
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

