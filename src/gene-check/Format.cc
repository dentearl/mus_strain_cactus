#include "Format.h"
#include <iostream>
#include <stdio.h>
#include <string.h>

/*
 * Do a sprintf style formating to a string.  This has no
 * fixed limit on the string size.
 */
string Format::vformat(const char* formatStr, 
                       va_list args) {
    string buf;
    
    // guess at initialize size
    buf.resize(strlen(formatStr)*4);

    // loop, growing buffer as needed, until format succeeds. 
    // cheat, grabbing memory of string and casting.  Note that
    // some snprintf implementations return -1, others > size
    // on failure.
    while (true) {
        int fmtSize = vsnprintf(const_cast<char*>(buf.c_str()),
                                buf.size()-1, formatStr, args);
        if ((fmtSize < 0) || (fmtSize > (buf.size()-1))) {
            buf.resize(buf.size()*2);
        } else {
            // adjust size to match C string
            buf.resize(strlen(buf.c_str()));
            break;
        }
    } 
    return buf;
}

/*
 * Do a sprintf style formating to a string.  This has no
 * fixed limit on the string size.
 */
string Format::format(const char* formatStr,
                      ...) {
    va_list args;
    va_start(args, formatStr);
    string buf = vformat(formatStr, args);
    va_end(args);
    return buf;
}

/*
 * Break a string into lines and print it, indenting each line.
 */
void Format::printLinesIndented(ostream& out,
                                const string& str,
                                int indent) {
    int idx = 0;
    int len = str.size();
    while (idx < len) {
        for (int cnt = 0; cnt < indent; cnt++) {
            out << ' ';
        }
        while ((idx < len) && (str[idx] != '\n')) {
            out << str[idx++];
        }
        out << endl;
        idx++;
    }
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */


