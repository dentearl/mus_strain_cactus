#include "CoordRangeMap.h"

extern "C" {
#define private fprivate
#include "common.h"
#include "hash.h"
#include "binRange.h"
#undef private
#undef bool
}

static const int HASH_POW2_SIZE = 8;

/** Constructor */
CoordRangeMapImpl::CoordRangeMapImpl(Coords::System coordSys,
                                     void (*freeFunc)(void**)):
    fCoordSys(coordSys),
    fRangeIndex(hashNew(HASH_POW2_SIZE)),
    fFreeFunc(freeFunc) {
}

/* Destructor */
CoordRangeMapImpl::~CoordRangeMapImpl() {
    if (fFreeFunc != NULL) {
        freeObjs();
    }
    freeBins();
}

/*
 * Free all bins
 */
void CoordRangeMapImpl::freeBins() {
    struct hashCookie cookie = hashFirst(fRangeIndex);
    struct hashEl* hel;
    while ((hel = hashNext(&cookie)) != NULL) {
        struct binKeeper *bk = static_cast<struct binKeeper*>(hel->val);  // casting in call generated warning in some versions of gcc
        binKeeperFree(&bk);
    }
    hashFree(&fRangeIndex);
}

/*
 * Free all objects.
 */
void CoordRangeMapImpl::freeObjs() {
    assert(fFreeFunc != NULL);
    struct hashCookie cookie = hashFirst(fRangeIndex);
    struct hashEl* hel;
    while((hel = hashNext(&cookie)) != NULL) {
        struct binKeeper* bins = (struct binKeeper*)hel->val;
        for (int iBin = 0; iBin < bins->binCount; iBin++) {
            for (struct binElement* bin = bins->binLists[iBin];
                 bin != NULL; bin = bin->next) {
                (*fFreeFunc)(&bin->val);
            }
        }
    }
}

/*
 * Get the key to use indexing the table; either chrom or chrom with strand.
 */
string CoordRangeMapImpl::getChromKey(const Coords& coords) const {
    assert(coords.getSystem() == fCoordSys);
    if (fCoordSys == Coords::GENOMIC) {
        return coords.getName();
    } else {
        string key(coords.getName());
        char strand[3];
        strand[0] = ' ';
        strand[1] = coords.getStrand();
        strand[2] = '\0';
        key += strand;
        return key;
    }
}

/**
 * Get bins for a sequence, or null if not set.
 */
struct binKeeper* CoordRangeMapImpl::findBins(const string& chromKey) {
    return (struct binKeeper*)hashFindVal(fRangeIndex, (char*)chromKey.c_str());
}

/**
 * Get bins for a sequence, creating if needed
 */
struct binKeeper* CoordRangeMapImpl::getBins(const string& chromKey) {
    struct binKeeper* bins = findBins(chromKey);
    if (bins == NULL) {
        bins = binKeeperNew(0, 300000000);
        hashAdd(fRangeIndex, (char*)chromKey.c_str(), bins);
    }
    return bins;
}

/**
 * Add an object.
 */
void CoordRangeMapImpl::add(const Coords& range,
                            void* obj) {
    binKeeperAdd(getBins(getChromKey(range)), range.getStart(), range.getEnd(), obj);
}

/**
 * Get overlaping objects.
 */
void CoordRangeMapImpl::getOverlaping(const Coords& range,
                                      vector<void*>& objs) {
    struct binKeeper* bins = findBins(getChromKey(range));
    if (bins != NULL) {
        struct binElement *entries
            = binKeeperFind(bins, range.getStart(), range.getEnd());
        for (struct binElement *entry = entries; entry != NULL;
             entry = entry->next) {
            objs.push_back(entry->val);
        }
        slFreeList(&entries);
    }
}

/**
 * Get all objects.
 */
void CoordRangeMapImpl::getAll(vector<void*>& objs) {
    struct hashCookie cookie = hashFirst(fRangeIndex);
    struct hashEl* hel;
    while((hel = hashNext(&cookie)) != NULL) {
        struct binKeeper* bins = (struct binKeeper*)hel->val;
        for (int iBin = 0; iBin < bins->binCount; iBin++) {
            for (struct binElement* bin = bins->binLists[iBin];
                 bin != NULL; bin = bin->next) {
                objs.push_back(bin->val);
            }
        }
    }
}

/**
 * Free all entries.
 */
void CoordRangeMapImpl::clear() {
    if (fFreeFunc != NULL) {
        freeObjs();
    }
    freeBins();
    fRangeIndex = hashNew(HASH_POW2_SIZE);
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

