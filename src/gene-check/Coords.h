#ifndef COORDS_H
#define COORDS_H

using namespace std;
#include <typeinfo>
#include <string>
#include <vector>
#include <iostream>
#include <assert.h>

/**
 * Defines a coordinates in a sequence. Addressing is either genomic (positive
 * strand) coordindates, or strand-specific.
 */
class Coords {
 public:
    static const char POS_STRAND = '+';
    static const char NEG_STRAND = '-';
    static const char NO_STRAND = '\0';

    /** Coordinate system */
    typedef enum {
        GENOMIC = 0,
        STRAND = 1
    } System;

 private:
    /** Chromosome or sequence name.  Empty for null position */
    string fName;

    /* coordinate system */
    System fSystem;

    /* strand we are associated with */
    char fStrand;

    /**  
     * Zero-base.  A zero length marker for a
     * position can be represented by fStart = fEnd-1.  That is the
     * point between the position and previous position.
     */
    unsigned fStart;
    unsigned fEnd;

    /** Length of the sequence, or 0 if not available */
    unsigned fSeqSize;

    /** assert that the coordinate systems are the same */
    void assertSameSys(const Coords& other) const {
        assert(isNull() || other.isNull() || (other.getSystem() == fSystem));
    }

    /** assert that the name and coordinate systems are the same */
    void assertSameNameSys(const Coords& other) const {
        assert(other.getName() == fName);
        assertSameSys(other);
    }

 public:
    /** NULL coordinate */
    static const Coords NULL_COORD;

    /** Constructor for a null position */
    Coords(): 
        fSystem(GENOMIC),
        fStrand(NO_STRAND),
        fStart(0),
        fEnd(0),
        fSeqSize(0) {
    }

    /** Constructor */
    Coords(const string& name,
           System system,
           char strand,
           unsigned start,
           unsigned end,
           unsigned seqLen) :
        fName(name),
        fSystem(system),
        fStrand(strand),
        fStart(start),
        fEnd(end),
        fSeqSize(seqLen) {
        assertValid();
    }

    /** 
     * Constructor, using another Coords to get attributes
     */
    Coords(const Coords& src,
           unsigned start,
           unsigned end) :
        fName(src.fName),
        fSystem(src.fSystem),
        fStrand(src.fStrand),
        fStart(start),
        fEnd(end),
        fSeqSize(src.fSeqSize) {
        assertValid();
    }

    /** 
     * Constructor, using another Coords to get attributes,
     * specifying strand.  If system is STRAND, then
     * start/end must be on correct strand.
     */
    Coords(const Coords& src,
           char strand,
           unsigned start,
           unsigned end) :
        fName(src.fName),
        fSystem(src.fSystem),
        fStrand(strand),
        fStart(start),
        fEnd(end),
        fSeqSize(src.fSeqSize) {
        assertValid();
    }

    /** 
     * Copy constructor, perhaps changing coordinate system.
     */
    Coords(const Coords& src,
           System system):
        fName(src.fName),
        fSystem(system),
        fStrand(src.fStrand),
        fSeqSize(src.fSeqSize) {
        if (system == src.fSystem) {
            fStart = src.fStart;
            fEnd = src.fEnd;
        } else {
            fStart = (src.fStrand == POS_STRAND) ? src.fStart
                : (fSeqSize - src.fEnd);
            fEnd = (src.fStrand == POS_STRAND) ? src.fEnd
                : (fSeqSize - src.fStart);
        }
        assertValid();
    }

    /* Convert to the specified coordinate system */
    Coords toSystem(System system) const {
        return Coords(*this, system);
    }

    /* Convert to genomic coordinate system */
    Coords toGenomic() const {
        return Coords(*this, GENOMIC);
    }

    /* Convert to strand coordinate system */
    Coords toStrand() const {
        return Coords(*this, STRAND);
    }

    /** Sanity check on the class */
    void assertValid() const {
        assert((fStrand == POS_STRAND) || (fStrand == NEG_STRAND)
               || (fStrand == NO_STRAND));
        assert((fSystem == GENOMIC) || (fSystem == STRAND));

        // only null can have zero-length sequence
        assert(((fName.size() == 0) && (fSeqSize == 0))
               || ((fName.size() > 0) && (fSeqSize > 0)));

        // allow zero-length
        assert(fStart <= fEnd);
    }

    /** 
     * Check if coordinates are compatible. Must have the same coordinate
     * system and name, and if genomic, the same strand.
     */
    bool compatible(const Coords& other) const {
        return (fName == other.fName) && (fSystem == other.fSystem)
            && ((fSystem == GENOMIC) || (fStrand == other.fStrand));
    }

    /** Get the name */
    const string& getName() const {
        return fName;
    }

    /* Get the coordinate system */
    System getSystem() const {
        return fSystem;
    }

    /** Get the strand */
    char getStrand() const {
        return fStrand;
    }

    /** Get the strand as a direction */
    int getDirection() const {
        // no strand is forward
        return (fStrand == NEG_STRAND) ? -1 : 1;
    }

    /** Get the start position */
    unsigned getStart() const {
        return fStart;
    }
    /** Get the end position */
    unsigned getEnd() const {
        return fEnd;
    }

    /** Get the length */
    unsigned getLength() const {
        return (fEnd - fStart);
    }

    /** Get the sequence length */
    unsigned getSeqSize() const {
        return fSeqSize;
    }

    /* Is this a null position */
    bool isNull() const {
        return fName.size() == 0;
    }

    /** Get the start as a Coords */
    Coords getStartCoords() const {
        return Coords(fName, fSystem, fStrand, fStart, fStart+1, fSeqSize);
    }

    /** Get the end as a Coords */
    Coords getEndCoords() const {
        return Coords(fName, fSystem, fStrand, fEnd-1, fEnd, fSeqSize);
    }

    /* Return a new coords with start and stop incremented */
    Coords getIncr(int amt = 1) const {
        return Coords(fName, fSystem, fStrand, fStart+amt, fEnd+amt, fSeqSize);
    }

    /* equals operator */
    bool operator==(const Coords& other) const {
        return (fName == other.fName)
            && (fSystem == other.fSystem)
            && (fStrand == other.fStrand)
            && (fStart == other.fStart)
            && (fEnd == other.fEnd)
            && (fSeqSize == other.fSeqSize);
    }

    /* not equals operator */
    bool operator!=(const Coords& other) const {
        return !(*this == other);
    }

    /** Are the coordinate systems the same */
    bool sameSystem(const Coords& other) const {
        return (fSystem == other.getSystem());
    }

    /** Determine if the names are the same. */
    bool sameName(const Coords& other) const {
        return (fName == other.getName());
    }

    /** Determine if the strands are the same. */
    bool sameStrand(const Coords& other) const {
        return (fStrand == other.fStrand);
    }

    /** Determine if the name and strand are the same. */
    bool sameNameStrand(const Coords& other) const {
        return sameName(other) && sameStrand(other);
    }

    /** Check if this is the negative strand */
    bool isNegStrand() const {
        return (fStrand == NEG_STRAND);
    }

    /** Check if this is the positive or no strand */
    bool isNonNegStrand() const {
        return (fStrand != NEG_STRAND);
    }

    /** 
     * Compare two coordinates.  Negative strand is considered less than
     * the positive strand if strand coordinates
     */
    int compare(const Coords& other) const {
        assertSameSys(other);

        // name first
        int cmp = fName.compare(other.fName);
        if (cmp != 0) {
            return cmp;
        }
        
        // Handle different strands
        if ((fSystem == STRAND) && (other.getStrand() != fStrand)) {
            if (fStrand == POS_STRAND) {
                return 1;  // this greater
            } else{
                return -1;  // other greater
            }
        }

        if (fStart > other.getStart()) {
            return 1;  // this greater
        } else if (fStart < other.getStart()) {
            return -1;  // other greater
        } 
        if (fEnd > other.getEnd()) {
            return 1;  // this greater
        } else if (fEnd < other.getEnd()) {
            return -1;  // other greater
        } else {
            return 0; // equal!
        }
    }

    /** Check if a range overlaps */
    bool overlaps(const Coords& other) const {
        assertSameSys(other);
        if (!(sameName(other)
            && ((sameStrand(other) || (fSystem == GENOMIC))))) {
            return false;
        } else {
            return ((other.getStart() < getEnd())
                    && (other.getEnd() > getStart()));
        }
    }

    /** Get the overlapping range */
    Coords getOverlap(const Coords& other) const;

    /** Determine by how many positions two ranges overlap  */
    int overlapAmount(const Coords& other) const;

    /** Compute what fraction of this range that is overlapped by another */
    double overlapFrac(const Coords& other) const;

    /** Check if a position is contained in this range */
    bool contains(int pos) const {
        return (pos >= fStart) && (pos < fEnd);
    }

    /** Check if a range is contained in this range */
    bool contains(const Coords& other) const {
        assertSameSys(other);
        return (other.fName == fName)
            && (other.fStrand == fStrand)
            && (other.fStart >= fStart)
            && (other.fEnd <= fEnd);
    }

    /** convert to a string */
    string toString() const;
};

/**
 * Coords output operator.
 */
inline ostream& operator<<(ostream &out,
                           const Coords &coords) {
    out << coords.toString();
    return out;
} 

/* Vector of coordinates */
typedef vector<Coords> CoordVector;

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

