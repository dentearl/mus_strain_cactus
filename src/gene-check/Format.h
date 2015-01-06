#ifndef FORMAT_H
#define FORMAT_H

#include <string>
#include <stdarg.h>
using namespace std;
class StringVector;

/**
 * Formatting methods.
 *
 * @author Mark Diekhans &lt;markd@cse.ucsc.edu&gt;
 */
class Format {
private:
    /*
     * Private constructor so instances can't be created.
     */
    Format() {
    }

public:
    /**
     * Do a sprintf style formating to a string.  This has no
     * fixed limit on the string size.
     *
     * @param formatStr fprint-style format string for messages.
     * @param args Remaining arguments are formatted into the message.
     */
    static string vformat(const char* formatStr, 
                          va_list args);

    /**
     * Do a sprintf style formating to a string.  This has no
     * fixed limit on the string size.
     *
     * @param formatStr fprint-style format string for messages.
     * @param ... Remaining arguments are formatted into the message.
     */
    static string format(const char* formatStr,
                         ...);

    /**
     * Break a string into lines and print it, indenting each line.
     */
    static void printLinesIndented(ostream& out,
                                   const string& str,
                                   int indent = 0);
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */


