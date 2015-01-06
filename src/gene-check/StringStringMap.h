/*
 * FILE: StringStringMap.h
 * AUTHOR: Mark Diekhans <markd@cse.ucsc.edu>
 * CREATE DATE: 3/11/1999
 * PROJECT: G-Known
 * DESCRIPTION: Hash table of string to string.
 * VERSION: $Revision$
 *
 * Copyright 1998-1999, The Regents of the University of California
 *
 * Departments of Computer Engineering and Computer Science
 * Jack Baskin School of Engineering
 * University of California, Santa Cruz, CA 95064
 */

#ifndef STRING_STRING_HASH_TABLE_H
#define STRING_STRING_HASH_TABLE_H

#include <typeinfo>
#include <string>
#include <map>
#include "FatalError.h"
#include "Generator.h"

/**
 * Map of string to string.
 *
 * @author Mark Diekhans &lt;markd@cse.ucsc.edu&gt;
 */
class StringStringMap: public map<const string, const string> {
  public:
    typedef map<const string, const string> SuperType;  // saves some typing

    /** Key/value pair */
    typedef pair<const string, const string> Pair;
    typedef SuperType::const_iterator const_iterator;
    typedef SuperType::iterator iterator;

    /** Insert an object, keeping existing one, use [] to replace */
    iterator insert(const string& key,
                    const string& value) {
        return SuperType::insert(Pair(key, value)).first;
    }

    /** remove an object */
    void remove(const string& key) {
        SuperType::erase(key);
    }

    /** Is a key contained in this object */
    bool contains(const string& key) const {
        return (SuperType::find(key) != SuperType::end());
    }

    /** get an pointer to an entry or NULL */
    const string* get(const string& key) const {
        SuperType::const_iterator hit = SuperType::find(key);
        if (hit == SuperType::end()) {
            return NULL;
        } else {
            return &hit->second;
        }
    }

    /**
     * Get a generator over all entries in the table.
     */
    ConstGenerator<SuperType> getEntries() const {
        return ConstGenerator<SuperType>(SuperType::begin(), SuperType::end());
    }
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

