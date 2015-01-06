#ifndef STRING_OPS_H
#define STRING_OPS_H

#include <string>
#include <string.h>
#include "StringVector.h"

/**
 * Operations on strings
 */
class StringOps {
 public:
    /**
     * String containing the whitespace characters.
     */
    static const string WHITE_SPACE;

    /**
     * An empty string.
     */
    static const string EMPTY;

    /**
     * Trim trailing blanks from a string.
     */
    static string trimTrailingBlanks(const string& str);

    /**
     * Trim leading and training blanks from a string.
     */
    static string trimBlanks(const string& str);

    /**
     * Pad a string with blanks on the left to the specified width
     */
    static string padLeft(const string& str,
                          int width);

    /**
     * Pad a string with blanks on the r8ight to the specified width
     */
    static string padRight(const string& str,
                           int width);

    /**
     * Determine if a string constains white space.
     */
    static bool containsSpaces(const string& str);

    /**
     * Determine if a string starts with a prefix, followed by whitespace
     * or end-of-string
     */
    static bool startsWith(const string& prefixStr,
                           const string& str) {
        if (prefixStr.size() > str.size()) {
            return false;
        } else {
            return str.compare(0, prefixStr.size(), prefixStr, 0, prefixStr.size()) == 0;
        }
    }

    /**
     * Determine if a string starts with a prefix
     */
    static bool startsWithWord(const string& prefixStr,
                               const string& str) {
        return startsWith(prefixStr, str)
            && ((prefixStr.size() == str.size()) || isspace(str[prefixStr.size()]));
    }

    /**
     * Determine if a string starts with a prefix.
     */
    static bool prefix(const string& prefixStr,
                       const string& str) {
        return startsWith(prefixStr, str);
    }

    /**
     * Determine if a string ends with a suffix
     */
    static bool endsWith(const string& str, const string& suffix) {
        if (suffix.size() > str.size()) {
            return false;
        } else {
            return str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
        }
    }

    /**
     * Compare two C strings for equality.
     */
    static inline bool strequ(const char* str1,
                              const char* str2) {
        return (strcmp(str1, str2) == 0);
    }

    /**
     * Search a NULL terminated array of C strings for a C string.
     */
    static bool contains(const char* str,
                         const char** strArray);

    /**
     * Search a NULL terminated array of C strings for a string.
     */
    static bool contains(const string& str,
                         const char** strArray);

    /**
     * Convert a string to upper case.
     */
    static string toUpper(const string& str);

    /**
     * Convert all or part of a string to upper case.
     */
    static void shiftToUpper(string& str,
                             int pos = 0,
                             int npos = -1);

    /**
     * Convert a string to lower case.
     */
    static string toLower(const string& str);

    /**
     * Convert all or part of a string to lower case.
     */
    static void shiftToLower(string& str,
                             int pos = 0,
                             int npos = -1);


    /** String quality, ignoring case. */
    static bool equalIgnoreCase(const string& str1,
                                const string& str2) {
        int len = str1.size();
        if (str2.size() != len) {
            return false;
        }
        for (int i = 0; i < len; i++) {
            if (tolower(str1[i]) != tolower(str2[i])) {
                return false;
            }
        }
        return true;
    }

    /**
     * Reverse a string.
     */
    static void reverse(string& str);

    /**
     * Find the index of the first mismatched character.
     */
    static int firstMismatch(const string& str1,
                             const string& str2,
                             int off = 0);

    /**
     * Get the minimum size of two strings.
     */
    static int minSize(const string& str1,
                       const string& str2) {
        return (str1.size() < str2.size()) ? str1.size() : str2.size();
    }

    /**
     * Get the maximum size of two strings.
     */
    static int maxSize(const string& str1,
                       const string& str2) {
        return (str1.size() > str2.size()) ? str1.size() : str2.size();
    }

    /**
     * Create a string from n-copies of a character.
     */
    static string replicate(int cnt,
                            char ch = ' ');

    /**
     * Create a string from n-copies of a string.
     */
    static string replicate(int cnt,
                            const string& srcStr);

    /**
     * Substitute strings into a template.  This is similar to sprintf, taking
     * subtitution specs in the form `%c' and replacing them with a string.
     * The set of characters for the tokes is supplied, along with the values.
     *
     * @param tmplStr String to substitute tokens into
     * @param tokens List of token characters
     * @param tokenVals  Values to subsitute for the corresponding token char.
     */
    static string substToken(const string& tmplStr,
                             const string& tokens,
                             const string* const* tokenVals);


    /**
     * Calculate an integer hash code for a string.
     * Taken from Tcl generic/tclHash.c
     */
    static unsigned calculateHashCode(const string& str) {
        unsigned int result = 0;
        for (int i = 0; i < str.size(); i++) {
            result += (result<<3) + str[i];
        }
        return result;
    }
};
#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
