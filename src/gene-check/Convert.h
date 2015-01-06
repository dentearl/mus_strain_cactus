
#ifndef CONVERT_H
#define CONVERT_H

#include "Types.h"
#include <string>
using namespace std;

/** Conversions of types to/from strings. */
class Convert {
  public:
    /** Convert an integer to a string. */
    static string toString(int num);

    /**
     * Convert a int to a string, right justified to take up
     * no more than the specified number of characters.
     * @zeroPad - if true, then pad with zeros rather than spaces.
     */
    static string toFixedWidthString(int num,
                                     int strWidth,
                                     bool zeroPad=false);

    /** Convert an unsigned to a string. */
    static string toString(unsigned num);

    /** Convert an unsigned long to a string. */
    static string toString(unsigned long num);

    /** Convert a long long to a string. */
    static string toString(long long num);

    /** Convert a string to an int. */
    static int toInt(const char* str,
                     bool* isOk = NULL,
                     int base = 0);

    /** Convert a string to an int. */
    static int toInt(const string& str,
                     bool* isOk = NULL,
                     int base = 0) {
        return toInt(str.c_str(), isOk, base);
    }

    /** Convert a string to an unsigned. */
    static unsigned toUnsigned(const char* str,
                               bool* isOk = NULL,
                               int base = 0);

    /** Convert a string to an unsigned. */
    static unsigned toUnsigned(const string& str,
                               bool* isOk = NULL,
                               int base = 0) {
        return toUnsigned(str.c_str(), isOk, base);
    }

    /* Convert a string to a long long. */
    static long long toLongLong(const char* str,
                                bool* isOk = NULL);


    /* Convert a string to a long long. */
    static long long toLongLong(const string& str,
                                bool* isOk = NULL) {
        return toLongLong(str.c_str(), isOk);
    }

    /** Convert a float to a string. */
    static string toString(float num,
                           int precision = 6);

    /** Convert a double to a string. */
    static string toString(double num,
                           int precision = 6);

    /**
     * Convert a double to a string, right justified to take up
     * no more than the specified number of characters.
     */
    static string toFixedWidthString(double num,
                                     int strWidth,
                                     int precision = 6);

    /** Convert a string to a float. */
    static float toFloat(const char* str,
                         bool* isOk = NULL);

    /** Convert a string to a float. */
    static float toFloat(const string& str,
                         bool* isOk = NULL) {
        return toFloat(str.c_str(), isOk);
    }

    /** Convert a string to a double. */
    static double toDouble(const char* str,
                           bool* isOk = NULL);

    /** Convert a string to a double. */
    static double toDouble(const string& str,
                           bool* isOk = NULL) {
        return toDouble(str.c_str(), isOk);
    }

    /** Convert a bool to a string. */
    static string toString(bool val);

    /**
     * Parse a boolean value.  Values of "true", "false", "on", "off",
     * "yes" and "no" are accepted.  Case is not signficant.
     */
    static bool toBool(const char* str,
                       bool* isOk = NULL);

    /**
     * Parse a boolean value.  Values of "true", "false", "on", "off",
     * "yes" and "no" are accepted.  Case is not signficant.
     */
    static bool toBool(const string& str,
                       bool* isOk = NULL) {
        return toBool(str.c_str(), isOk);
    }

    /** Convert a character to a string. */
    static string toString(char val);

    /** Convert a unsigned character to a string. */
    static string toString(unsigned char val);
};
#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */
