#include "StringOps.h"
#include "FatalError.h"

/**
 * String containing the whitespace characters (as defined by isspace()).
 */
const string StringOps::WHITE_SPACE = "\011\012\013\014\015\040";

/**
 * An empty string.
 */
const string StringOps::EMPTY;

/*
 * Trim trailing blanks from a string.
 */
string StringOps::trimTrailingBlanks(const string& str) {
    // Find last non-blank
    size_t lastIdx = str.size()-1;
    while ((lastIdx > 0) && isspace(str[lastIdx])) {
        lastIdx--;
    }

    // avoid substr if not needed to share underlying string
    if (lastIdx < str.size()-1) {
        return str.substr(0, lastIdx+1);
    } else {
        return str;
    }
}

/*
 * Trim leading and training blanks from a string.
 */
string StringOps::trimBlanks(const string& str) {
    // Find first non-blank.
    size_t firstIdx = 0;
    while ((firstIdx < str.size()) && isspace(str[firstIdx])) {
        firstIdx++;
    }
    
    // Find last non-blank
    size_t lastIdx = str.size()-1;
    while ((lastIdx > firstIdx) && isspace(str[lastIdx])) {
        lastIdx--;
    }

    // avoid substr if not needed to share underlying string
    if ((firstIdx > 0) || (lastIdx < str.size()-1)) {
        return str.substr(firstIdx, lastIdx-firstIdx+1);
    } else {
        return str;
    }
}

/**
 * Pad a string with blanks on the left to the specified width
 */
string StringOps::padLeft(const string& str,
                          int width) {
    string outStr;
    for (int cnt = str.size(); cnt < width; cnt++) {
        outStr += ' ';
    }
    outStr += str;
    return outStr;
}

/**
 * Pad a string with blanks on the right to the specified width
 */
string StringOps::padRight(const string& str,
                           int width) {
    string outStr(str);
    while (outStr.size() < width) {
        outStr += ' ';
    }
    return outStr;
}

/*
 * Determine if a string constains white space.
 */
bool StringOps::containsSpaces(const string& str) {
    int len = str.size();
    for (int idx = 0; idx < len; idx++) {
        if (isspace(str[idx])) {
            return true;
        }
    }
    return false;
}

/**
 * Search a NULL terminated array of C strings for a C string.
 */
bool StringOps::contains(const char* str,
                         const char** strArray) {
    for (int idx = 0; strArray[idx] != NULL; idx++) {
        if (strequ(str, strArray[idx])) {
            return true;
        }
    }
    return false;
}

/**
 * Search a NULL terminated array of C strings for a string.
 */
bool StringOps::contains(const string& str,
                         const char** strArray) {
    return contains(str.c_str(), strArray);
}

/**
 * Convert a string to upper case.
 */
string StringOps::toUpper(const string& str) {
    int len = str.size();
    string newStr;
    newStr.resize(len);
    for (int idx = 0; idx < len; idx++) {
        newStr[idx] = toupper(str[idx]);
    }
    return newStr;
}

/**
 * Convert a string to upper case in place.
 */
void StringOps::shiftToUpper(string& str,
                             int pos,
                             int npos) {
    int lpos = (pos+npos)-1;
    if (lpos < pos) {
        lpos = str.size()-1;
    }
    for (; pos <= lpos; pos++) {
        str[pos] = toupper(str[pos]);
    }
}

/**
 * Convert a string to lower case.
 */
string StringOps::toLower(const string& str) {
    int len = str.size();
    string newStr;
    newStr.resize(len);
    for (int idx = 0; idx < len; idx++) {
        newStr[idx] = tolower(str[idx]);
    }
    return newStr;
}

/**
 * Convert a string to lower case in place.
 */
void StringOps::shiftToLower(string& str,
                             int pos,
                             int npos) {
    int lpos = (pos+npos)-1;
    if (lpos < pos) {
        lpos = str.size()-1;
    }
    for (; pos <= lpos; pos++) {
        str[pos] = tolower(str[pos]);
    }
}

/**
 * Reverse a string.
 */
void StringOps::reverse(string& str) {
    int fIdx = 0;
    int bIdx = str.size()-1;
    
    while (fIdx < bIdx) {
        char hold = str[fIdx];
        str[fIdx] = str[bIdx];
        str[bIdx] = hold;
        fIdx++;
        bIdx--;
    }
}

/**
 * Find the index of the first mismatched character.
 */
int StringOps::firstMismatch(const string& str1,
                             const string& str2,
                             int off) {
    int minLen = (str1.size() < str2.size()) ? str1.size() : str2.size();
    for (int idx = off ; idx < minLen; idx++) {
        if (str1[idx] != str2[idx]) {
            return idx;
        }
    }
    if (str1.size() != str2.size()) {
        return minLen;
    } else {
        return -1;
    }
}

/**
 * Create a string from n-copies of a character.
 */
string StringOps::replicate(int cnt,
                            char ch) {
    string str;
    str.resize(cnt);
    for (int idx = 0; idx < cnt; idx++) {
        str[idx] = ch;
    }
    return str;
}

/**
 * Create a string from n-copies of another string.
 */
string StringOps::replicate(int cnt,
                            const string& srcStr) {
    string str;
    for (int idx = 0; idx < cnt; idx++) {
        str += srcStr;
    }
    return str;
}

/**
 * Substitute strings into a template.
 */
string StringOps::substToken(const string& tmplStr,
                             const string& tokens,
                             const string* const* tokenVals) {
    int tmplStrLen = tmplStr.size();
    string str;
    for (int i = 0; i < tmplStrLen; i++) {
        if (tmplStr[i] == '%') {
            i++;
            if (i == tmplStrLen) {
                throw FatalError("template string ends in`%': " + tmplStr);
            }
            if (tmplStr[i] == '%') {
                str += '%'; // quoted %
            } else {
                size_t iToken = tokens.find(tmplStr[i]);
                if (iToken == string::npos) {
                    throw FatalError(string(" unknown token `%")
                                     + tmplStr[i] + "' in template: "
                                     + tmplStr);
                }
                str += *(tokenVals[iToken]);
            }
        } else {
            str += tmplStr[i];
        }
    }
    return str;
}

/*
 * Local Variables:
 * mode: c++
 * End:
 */
