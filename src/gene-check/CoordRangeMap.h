#ifndef COORD_RANGE_MAP_H
#define COORD_RANGE_MAP_H

using namespace std;
#include <typeinfo>
#include <vector>
#include "Coords.h"
#include "FatalError.h"
struct binKeeper;
struct hash;

/* Concrete class that hides the implemention details, yet allows
 * this to be a template.*/
class CoordRangeMapImpl {
    private:
    /* coordinate system */
    Coords::System fCoordSys;

    /* Hash of binRange objects used to find overlaping ranges. */
    struct hash* fRangeIndex;

    /* Optional free function */
    void (*fFreeFunc)(void**);

    void freeBins();
    void freeObjs();
    string getChromKey(const Coords& coords) const;
    struct binKeeper* findBins(const string& chromKey);
    struct binKeeper* getBins(const string& chromKey);

    public:
    /* constructor */
    CoordRangeMapImpl(Coords::System coordSys,
                      void (*freeFunc)(void**) = NULL);

    /* destructor */
    ~CoordRangeMapImpl();

    /** Add an object. */
    void add(const Coords& range,
             void* obj);

    /** Get overlaping objects. */
    void getOverlaping(const Coords& range,
                       vector<void*>& objs);
    /** Get all objects. */
    void getAll(vector<void*>& objs);

    /** Free all entries. */
    void clear();
};

/**
 * Map of coordinate ranges to objects.  Wrapper around binKeeper to
 * support multiple chromsomes.  If coordinate system is strand, the
 * strands are treated separately.
 */
template <class OBJPTR>
class CoordRangeMap {
    private:
    /* implementation */
    CoordRangeMapImpl fImpl;

    public:
    /* 
     * Constructor. If free function is not null, it's called to delete
     * objects.  Takes a pointer to a variable containing the object.
     */
    CoordRangeMap(Coords::System coordSys,
                  void (*freeFunc)(OBJPTR*) = NULL):
        fImpl(coordSys, (void (*)(void**))freeFunc) {
    }

    /** Add an object. */
    void add(const Coords& range,
             OBJPTR obj) {
        fImpl.add(range, obj);
    }
     
    /** Get overlaping objects */
    void getOverlaping(const Coords& range,
                       vector<OBJPTR>& objs) {
        fImpl.getOverlaping(range, (vector<void*>&)objs);
    }

    /** Get all objects */
    void getAll(vector<OBJPTR>& objs) {
        fImpl.getAll((vector<void*>&)objs);
    }

    /** 
     * Clear the object. Will free objects if a freeFunction was supplied.
     */
    void clear();
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

