/*
 * FILE: StringVector.h
 * AUTHOR: Mark Diekhans <markd@cse.ucsc.edu>
 * CREATE DATE: 2/20/1999
 * PROJECT: G-Known
 * DESCRIPTION: Vector of strings.
 * VERSION: $Revision$
 *
 * Copyright 1998-1999, The Regents of the University of California
 *
 * Departments of Computer Engineering and Computer Science
 * Jack Baskin School of Engineering
 * University of California, Santa Cruz, CA 95064
 */

#include "StringVector.h"
#include <algorithm>

/**
 * Add another StringVector to this vector.
 */
void StringVector::add(const StringVector& strVec) {
    ConstIterator iter = strVec.begin();
    while (iter < strVec.end()) {
        push_back(*iter);
        iter++;
    }
}

/**
 * Add an argv to this vector.
 */
void StringVector::add(int argc,
                       const char * const *argv) {
    for (int idx = 0; idx < argc; idx++) {
        add((string)argv[idx]);
    }
}

/**
 * Join a string vector into a string given a separator character.
 */
string StringVector::join(char separator) const {
    string str;
    
    for (int idx = 0; idx < (int)size(); idx++) {
        if (idx > 0) {
            str += separator;
        }
        str += (*this)[idx];
    }
    return str;
}

/**
 * Join the string vector into a string given a separator string.
 */
string StringVector::join(const string& separator) const {
    string str;
    
    for (int idx = 0; idx < (int)size(); idx++) {
        if (idx > 0) {
            str += separator;
        }
        str += (*this)[idx];
    }
    return str;
}

/**
 * Search the array for a string.
 */
int StringVector::find(const string& str) const {
    int len = size();
    for (int idx = 0; idx < len; idx++) {
        if (operator[](idx) == str) {
            return idx;
        }
    }
    return -1;
}

/**
 * Search the array for a string.
 */
int StringVector::find(const char* str) const {
    int len = size();
    for (int idx = 0; idx < len; idx++) {
        if (operator[](idx) == str) {
            return idx;
        }
    }
    return -1;
}

/**
 * Does this object have the same values in the same order as another
 * StringVector.
 */
bool StringVector::equals(const StringVector& other) const {
    if (other.size() != size()) {
        return false;
    }
    for (unsigned i = 0; i < size(); i++) {
        if ((*this)[i] != other[i]) {
            return false;
        }
    }
    return true;
}

/**
 * Determine if a string is contained in the vector.
 */
bool StringVector::contains(const string& str) const {
    return (find(str) >= 0);
}

/**
 * Determine if a C string is contained in the vector.
 */
bool StringVector::contains(const char* str) const {
    return (find(str) >= 0);
}

/* 
 * Sort the object
 */
void StringVector::sort() {
    ::sort(begin(), end());
}

/*
 * Split a string into a vector of string given a separator character.
 */
StringVector StringVector::split(const string& str,
                                 char separator) {
    StringVector strs;
    
    int prevIdx = 0;
    int sepIdx;
    while ((sepIdx = (int)str.find_first_of(separator, prevIdx)) >= 0) {
        strs.push_back(str.substr(prevIdx, sepIdx-prevIdx));
        prevIdx = sepIdx+1;
    }
    strs.push_back(str.substr(prevIdx));

    return strs;
}

/*
 * Split a string into a vector of string, with one or more whitespaces
 * being a considered as single separator.
 */
StringVector StringVector::splitOnWhiteSpace(const string& str) {
    // FIXME: avoid conversion to C string.
    const char* cstr = str.c_str();
    StringVector strs;

    unsigned scanIdx = 0;;
    while (true) {
        // Skip white space
        while ((cstr[scanIdx] != '\0') && isspace(cstr[scanIdx])) {
            scanIdx++;
        }
        if (cstr[scanIdx] == '\0') {
            break; // end of string
        }
        // Find end of string
        unsigned startIdx = scanIdx;
        while ((cstr[scanIdx] != '\0') && !isspace(cstr[scanIdx])) {
            scanIdx++;
        }
        strs.push_back(str.substr(startIdx, scanIdx-startIdx));
    }
    return strs;
}

/**
 * create a vector from a NULL terminated list of C strings.
 */
StringVector StringVector::fromNullList(char **strs) {
    StringVector vec;
    for (int i = 0; strs[i] != NULL; i++) {
        vec.push_back(string(strs[i]));
    }
    return vec;
}
/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

