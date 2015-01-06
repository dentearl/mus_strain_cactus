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

#ifndef STRINGVECTOR_H
#define STRINGVECTOR_H
using namespace std;
#include <typeinfo>
#include <string>
#include <vector>
#include <assert.h>
#include "Convert.h"
#include "Exception.h"


/**
 * A vector of strings.
 */
class StringVector: public vector<string> {
 public:
    // Interator types.
    typedef vector<string>::iterator Iterator;
    typedef vector<string>::const_iterator ConstIterator;

    /**
     * Construct an empty vector.
     */
    StringVector() {
    }

    /**
     * Copy constructor.
     */
    StringVector(const StringVector& src) {
        add(src);
    }

    /**
     * subrance copy constructor
     */
    StringVector(const StringVector& src,
                 int idx, int len) {
        assert(idx+len <= src.size());
        for (int cnt = 0; cnt < len; cnt++) {
            add(src[idx+cnt]);
        }
    }

    /**
     * Construct an vector from an argv.
     */
    StringVector(int argc,
                 const char* const* argv) {
        add(argc, argv);
    }

    /* destructor */
    virtual ~StringVector() {
    }

    /**
     * Add a string to the vector.
     */
    void add(const string& str) {
        push_back(str);
    }

    /**
     * Add an int to the vector
     */
    void add(int value) {
        push_back(Convert::toString(value));
    }

    /**
     * Add an unsigned int to the vector
     */
    void add(unsigned value) {
        push_back(Convert::toString(value));
    }

    /**
     * Add a double to the vector
     */
    void add(double value) {
        push_back(Convert::toString(value));
    }

    /**
     * Add another StringVector to this vector.
     */
    void add(const StringVector& strVec);

    /**
     * Add an argv to this vector.
     */
    void add(int argc, 
             const char * const *argv);

    /**
     * Get an entry with bounds checking.
     */
    const string& get(int idx) const {
        if ((idx < 0) || (idx >= (int)size())) {
            throw Exception("StringVector index out-of-bounds");
        }
        return operator[](idx);
    }

    /**
     * Get an entry, converted to an integer.
     */
    int getInt(int idx) const {
        return Convert::toInt(get(idx));
    }

    /**
     * Get an entry, converted to a double.
     */
    double getDouble(int idx) const {
        return Convert::toDouble(get(idx));
    }

    /**
     * Join the string vector into a string given a separator character.
     */
    string join(char separator = ' ') const;

    /**
     * Join the string vector into a string given a separator string.
     */
    string join(const string& separator) const;

    /**
     * Search the array for a string.
     * @return index of the string, or -1 if not found
     */
    int find(const string& str) const;

    /**
     * Does this object have the same values in the same order as another
     * StringVector.
     */
    bool equals(const StringVector& other) const;

    /**
     * Search the array for a string.
     * @return index of the string, or -1 if not found
     */
    int find(const char* str) const;

    /**
     * Determine if a string is contained in the vector.
     */
    bool contains(const string& str) const;

    /**
     * Determine if a C string is contained in the vector.
     */
    bool contains(const char* str) const;

    /* Sort the object */
    void sort();

    /**
     * Split a string into a vector of string given a separator character,
     * constructing a new vector.
     */
    static StringVector split(const string& str,
                              char separator);

    /**
     * create a vector from a NULL terminated list of C strings.
     */
    static StringVector fromNullList(char **strs);

    /*
     * Split a string into a vector of string, with one or more whitespaces
     * being a considered as single separator.
     */
    static StringVector splitOnWhiteSpace(const string& str);
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

