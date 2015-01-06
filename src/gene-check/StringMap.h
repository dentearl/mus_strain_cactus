#ifndef StringMap_h
#define StringMap_h

#include <map>
#include <string>
using namespace std;
#include "Generator.h"

/**
 * Template for string-keyed map.
 *
 * To declare a Generator
 *    Generator<StringMap<Value*>::SuperType> entries
 */
template<class OBJ>
class StringMap: public map<string, OBJ> {
    public:
    typedef map<string, OBJ> SuperType;  // saves some typing

    /** Key/value pair */
    typedef pair<string, OBJ> Pair;
    typedef typename SuperType::const_iterator const_iterator;
    typedef typename SuperType::iterator iterator;

    /** Insert an object, keeping existing one, use [] to replace */
    iterator insert(const string& key,
                    OBJ obj) {
        return SuperType::insert(Pair(key, obj)).first;
    }

    /** remove an object */
    void remove(const string& key) {
        SuperType::erase(key);
    }

    /** Is a key contained in this object */
    bool contains(const string& key) const {
        return (SuperType::find(key) != SuperType::end());
    }

    /** get an entry or return non-found if not found */
    OBJ get(const string& key,
            OBJ notFound = NULL) const {
        typename SuperType::const_iterator hit = SuperType::find(key);
        if (hit == SuperType::end()) {
            return notFound;
        } else {
            return hit->second;
        }
    }

    /**
     * Get a generator over all entries in the table.
     */
    Generator<SuperType> getEntries() {
        return Generator<SuperType>(SuperType::begin(), SuperType::end());
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

