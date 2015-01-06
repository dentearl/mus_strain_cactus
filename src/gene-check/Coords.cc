#include "Coords.h"
#include "Convert.h"
#include "Math.h"

/** NULL coordinate */
const Coords Coords::NULL_COORD;

/** 
 * Get the overlapping range
 */
Coords Coords::getOverlap(const Coords& other) const {
    assertSameSys(other);
    if (sameName(other)
        && ((sameStrand(other) || (fSystem == GENOMIC)))) {
        unsigned maxStart = Math::max(fStart, other.getStart());
        unsigned minEnd = Math::min(fEnd, other.getEnd());
        if (maxStart <= minEnd) {
            return Coords(*this, maxStart, minEnd);
        }
    }
    return NULL_COORD;
}

/** Determine by how many positions two ranges overlap  */
int Coords::overlapAmount(const Coords& other) const {
    assertSameSys(other);
    if (sameName(other)
        && ((sameStrand(other) || (fStrand == GENOMIC)))) {
        unsigned maxStart = Math::min(fStart, other.getStart());
        unsigned minEnd = Math::min(fEnd, other.getEnd());
        int overlapLen = (minEnd - maxStart) + 1;
        if (overlapLen > 0) {
            return overlapLen;
        }
    }
    return 0;
}

/** 
 * Compute what fraction of this range that is overlapped by another
 */
double Coords::overlapFrac(const Coords& other) const {
        int len = getLength();
        if (len == 0) {
            return 0.0;
        } else {
            return ((double)overlapAmount(other))/(double)len;
        }
    }

/** convert to a string */
string Coords::toString() const {
    if (isNull()) {
        return "null";
    } else {
        string str = fName;
        if ((fSystem == STRAND) && (fStrand != NO_STRAND)) {
            char strand[2];
            strand[0] = fStrand;
            strand[1] = '\0';
            str += strand;
        }
        str += ":" + Convert::toString(fStart);
        if (fEnd > fStart) {
            str += "-" + Convert::toString(fEnd);
        }
        return str;
    }
}


/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

