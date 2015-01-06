
#include "Exception.h"
#include <stdlib.h>

/* 
 * Display exception and abort or exit, depending if GK_ABORT environment
 * variable is set.  Normally called by constructor unless noExit is
 * specified.
 */
void Exception::displayAndExit() {
    cerr << "Exception: " << errorDesc << "\n";
    if (getenv("GK_ABORT") != NULL) {
        abort();
    } else {
        exit(1);
    }
}

/**
 * Construct a new exception.
 */
Exception::Exception(const string& errDesc,
                     bool noExit):
    errorDesc(errDesc) {
    if (!noExit) {
        displayAndExit();
    }
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

