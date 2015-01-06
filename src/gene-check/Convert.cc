#include "Convert.h"
#include "Exception.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>

// Constants for booleans
static const string TRUE_STR("true");
static const string FALSE_STR("false");

/*
 * Convert an integer to a string.
 */
string Convert::toString(int num) {
    char buf[64];
    sprintf(buf, "%d", num);
    return string(buf);
}

/*
 * Convert an unsigned to a string.
 */
string Convert::toString(unsigned num) {
    char buf[64];
    sprintf(buf, "%u", num);
    return string(buf);
}

/**
 * Convert a int to a string, right justified to take up
 * no more than the specified number of characters.
 */
string Convert::toFixedWidthString(int num,
                                   int strWidth,
                                   bool zeroPad) {
    char buf[strWidth + 128];

    sprintf(buf, (zeroPad ? "%.*d" : "%*d"), strWidth, num);
    return string(buf);
}

/*
 * Convert an unsigned long to a string.
 */
string Convert::toString(unsigned long num) {
    char buf[128];
    sprintf(buf, "%lu", num);
    return string(buf);
}

/*
 * Convert an long long to a string.
 */
string Convert::toString(long long num) {
    char buf[128];
    sprintf(buf, "%lld", num);
    return string(buf);
}

/*
 * Convert a string to an int.
 */
int Convert::toInt(const char* str,
                   bool* isOk,
                   int base) {
    char *endPtr;
    errno = 0;
    long lnum = strtol(str, &endPtr, base);
    if ((endPtr == str) || (*endPtr != '\0')) {
        if (isOk != NULL) {
            *isOk = false;
            return 0;
        } else {
            throw Exception("Invalid integer \"" + string(str) + "\"");
        }
    }
     
    int num = (int)lnum;
    if ((errno != 0) || ((long)num != lnum)) {
        if (isOk != NULL) {
            *isOk = false;
            return 0;
        } else {
            throw Exception("Integer out of range \"" + string(str) + "\"");
        }
    }
    if (isOk != NULL) {
        *isOk = true;
    }
    return num;
}

/*
 * Convert a string to an unsigned.
 */
unsigned Convert::toUnsigned(const char* str,
                             bool* isOk,
                             int base) {
    char *endPtr;
    errno = 0;
    unsigned long lnum = strtoul(str, &endPtr, base);
    if ((endPtr == str) || (*endPtr != '\0')) {
        if (isOk != NULL) {
            *isOk = false;
            return 0;
        } else {
            throw Exception("Invalid unsigned integer \"" + string(str) + "\"");
        }
    }
     
    unsigned num = (unsigned)lnum;
    if ((errno != 0) || ((unsigned long)num != lnum)) {
        if (isOk != NULL) {
            *isOk = false;
            return 0;
        } else {
            throw Exception("Unsigned integer out of range \"" + string(str) + "\"");
        }
    }
    if (isOk != NULL) {
        *isOk = true;
    }
    return num;
}

/*
 * Convert a string to a long long.
 */
long long Convert::toLongLong(const char* str,
                              bool* isOk) {
    char *endPtr;
    errno = 0;
    long long num = strtoll(str, &endPtr, 0);
    if ((endPtr == str) || (*endPtr != '\0')) {
        if (isOk != NULL) {
            *isOk = false;
            return 0;
        } else {
            throw Exception("Invalid long long \"" + string(str) + "\"");
        }
    }
    return num;
}

/*
 * Convert a float to a string.
 */
string Convert::toString(float num,
                         int precision) {
    /* Oddly, it's possible for a float to be smaller than FLT_MIN.  When
     * parsed with strtof this causes ERANGE to be set on FreeBSD but not on
     * Linux.  It's not clear what the right behavior is.  Values of FLT_MIN
     * also can cause error after truncation, so we just set to zero.
     */
    if ((num != 0.0) && (num >= -FLT_MIN) && (num <= FLT_MIN)) {
        num = 0.0;
    }
    char buf[64];
    sprintf(buf, "%.*g", precision, num);
    return string(buf);
}

/*
 * Convert a double to a string.
 */
string Convert::toString(double num,
                         int precision) {
    /* see explanation in float version */ 
    if ((num != 0.0) && (num >= -DBL_MIN) && (num <= DBL_MIN)) {
        num = 0.0;
    }
    char buf[64];
    sprintf(buf, "%.*g", precision, num);
    return string(buf);
}

/*
 * Convert a double to a string, right justified to take up
 * the specified number of characters (and no more).  Handles
 * special case of %g formatting in scientific notation will
 * produce more that the specified number of characters
 */
string Convert::toFixedWidthString(double num,
                                   int strWidth,
                                   int precision) {
    // n.b. sprintf field width is a minimum
    char buf[strWidth + 128];

    sprintf(buf, "%*.*g", strWidth, precision, num);
    if ((int)strlen(buf) > strWidth) {
        // Overflow, if scientific notation, don't lose exponent
        char* expStart = strchr(buf, 'e');
        if (expStart != NULL) {
            // Move exponent, if there is room, otherwise let string
            // overflow the width
            int expLen = strlen(expStart);
            if (expLen < strWidth-2) {
                strcpy(buf+(strWidth-expLen), expStart);
                buf[strWidth] = '\0';
            }
        } else {
            // just a long number
            buf[strWidth] = '\0';
        }
    }
    return string(buf);
}

/*
 * Convert a string to a float.
 */
float Convert::toFloat(const char* str,
                       bool* isOk) {
    char *endPtr;
    errno = 0;
    float num = strtof(str, &endPtr);
    if ((endPtr == str) || (*endPtr != '\0') || (errno != 0)) {
        if (isOk != NULL) {
            *isOk = false;
            return 0.0;
        } else if (errno != 0) {
            throw Exception("Float out of range \"" + string(str) + "\"");
        } else {
            throw Exception("Invalid float \"" + string(str) + "\"");
        }
    }
    if (isOk != NULL) {
        *isOk = true;
    }
    return num;
}

/*
 * Convert a string to a double.
 */
double Convert::toDouble(const char* str,
                         bool* isOk) {
    char *endPtr;
    errno = 0;
    double num = strtod(str, &endPtr);
    if ((endPtr == str) || (*endPtr != '\0') || (errno != 0)) {
        if (isOk != NULL) {
            *isOk = false;
            return 0.0;
        } else if (errno != 0) {
            throw Exception("Double out of range \"" + string(str) + "\"");
        } else {
            throw Exception("Invalid double \"" + string(str) + "\"");
        }
    }
    if (isOk != NULL) {
        *isOk = true;
    }
    return num;
}

/*
 * Convert a bool to a string.
 */
string Convert::toString(bool val) {
    return (val ? TRUE_STR : FALSE_STR);
}

/*
 * Parse a boolean value.  Values of "true", "false", "on", or "off" are
 * accepted.
 */
bool Convert::toBool(const char* str,
                     bool* isOk) {
    string strLower;

    // Down shift (#&^% string class doesn't do this.
    const char* src = str;
    while (*src != '\0') {
        strLower += *src++;
    }

    if (isOk != NULL) {
        *isOk = true;
    }
    if ((strLower == "true") || (strLower == "on") || (strLower == "yes")) {
        return true;
    } else if ((strLower == "false") || (strLower == "off") || (strLower == "no")) {
        return false;
    } else {
        if (isOk != NULL) {
            *isOk = false;
        } else {
            throw Exception("Invalid value for boolean: \"" + string(str) + "\"");
        }
    }
    return false;
}

/**
 * Convert a character to a string.
 */
string Convert::toString(char val) {
    char buf[2];
    buf[0] = val;
    buf[1] = '\0';
    return string(buf);
}

/**
 * Convert a unsigned character to a string.
 */
string Convert::toString(unsigned char val) {
    unsigned char buf[2];
    buf[0] = val;
    buf[1] = '\0';
    return string((char*)buf);
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

