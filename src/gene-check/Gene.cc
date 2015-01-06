
#include "Gene.h"
#include "FatalError.h"
#include "StringOps.h"

// FIXME: take errors out of here and move to geneCheck related object.
// FIXME: need better repersentation of exons vs CDS and utr.
// FIXME: also need to handle gaps differently.
// FIXME: CDS, UTR consts should be better on Gene than on Gene::Feature

/*
 * Feature type names.
 */
static const string UTR5_NAME = "utr5";
static const string CDS_NAME = "cds";
static const string INTRON_NAME = "intron";
static const string UTR3_NAME = "utr3";
static const string EXON_NAME = "exon";
static const string INTERGENIC_NAME = "intergenic";
static const string INVALID_NAME = "invalid";

/*
 * Exon type names.
 */
static const string NOT_EXON_NAME = "notexon";
static const string SINGLE_EXON_NAME = "single";
static const string INITIAL_EXON_NAME = "initial";
static const string INTERNAL_EXON_NAME = "internal";
static const string FINAL_EXON_NAME = "final";

/** 
 * Build the sequence for a feature from the sequence associated with the
 * gene.
 */
void Gene::Feature::buildSeq() {
    unsigned off = getSeqOff();
    unsigned len = getLength();
    assert((off+len) <= fGene->getSeq().size());
    fSeq = fGene->getSeq().substr(off, len);
}

/* Get start (5') splice site (if this is an intron), convert to
 * upper case. */
string Gene::Feature::getStartSplice() const {
    const string& seq = getSeq();
    string sp;
    if (seq.size() >= 2) {
        sp.resize(2);
        sp[0] = toupper(seq[0]);
        sp[1] = toupper(seq[1]);
    }
    return sp;
}

/* Get end (3') splice site (if this is an intron), convert to
 * upper case. */
string Gene::Feature::getEndSplice() const {
    const string& seq = getSeq();
    string sp;
    int sz = seq.size();
    if (sz >= 2) {
        sp.resize(2);
        sp[0] = toupper(seq[sz-2]);
        sp[1] = toupper(seq[sz-1]);
    }
    return sp;
}

/* 
 * Get the type as a string 
 * FIXME: really should use inheritance for this.
 */
const string& Gene::Feature::getTypeName() const {
    switch (fType) {
    case UTR5:
        return UTR5_NAME;
    case CDS:
        return CDS_NAME;
    case INTRON:
        return INTRON_NAME;
    case UTR3:
        return UTR3_NAME;
    case EXON:
        return EXON_NAME;
    case INTERGENIC:
        return INTERGENIC_NAME;
    default:
        assert(false);
        return INVALID_NAME;
    }
}

/* 
 * Get the exon type name.
 */
const string& Gene::Feature::getExonTypeName() const {
    switch (fExonType) {
    case NOT_EXON:
        return NOT_EXON_NAME;
    case SINGLE_EXON:
        return SINGLE_EXON_NAME;
    case INITIAL_EXON:
        return INITIAL_EXON_NAME;
    case INTERNAL_EXON:
        return INTERNAL_EXON_NAME;
    case FINAL_EXON:
        return FINAL_EXON_NAME;
    default:
        assert(false);
        return NOT_EXON_NAME;
    }
}

/** 
 * Constructor
 */
Gene::Gene(const string& name) :
    fName(name),
    fFirstRealFeatureIdx(0),
    fNumRealFeatures(0),
    fErrorFlags(0) {
    for (int i = 0; i < Feature::NUM_TYPES; i++) {
        fNumFeatures[i] = 0;
    }
}

/** 
 * Add a feature
 */
void Gene::addFeature(Feature::Type type,
                      const Coords& coords,
                      int frame) {
    assert(coords.getSystem() == Coords::STRAND);
    assert(coords.getLength() > 0);
    assert((-1 <= frame) && (frame <= 2));

    Feature feature(this, coords, type);
    if ((type == Feature::CDS) && (frame >= 0)) {
        feature.fStartFrame = frame;
        feature.fEndFrame = (frame + coords.getLength()) % 3;
    }
    // Find location to insert, searching backwards (since usually appending)
    int idx = fFeatures.size() - 1;
    while ((idx >= 0) && (feature.compare(fFeatures[idx]) < 0)) {
        idx--;
    }
    idx++; // insert at this coordition

    fFeatures.insert(fFeatures.begin()+idx, feature);

    // assert we got it in the right place.
    assert((idx == 0) || (fFeatures[idx-1].compare(fFeatures[idx]) < 0));
    assert((idx == (int)(fFeatures.size()-1))
           || (fFeatures[idx].compare(fFeatures[idx+1]) < 0));
}

/** 
 * Set preceeding intergenic.
 */
void Gene::setBeforeIntergenic(unsigned size) {
    assert(fFeatures.size() > 0);
    assert(fFeatures[0].getType() != Feature::INTERGENIC);

    // adjust before size to prevent running of start of chromosome
    const Coords& first = fFeatures[0];
    if (first.getStart() == 0) {
        size = 0;
    } else if (size >= first.getStart()) {
        size = first.getStart()-1;
    }
    if (size > 0) {
        Coords coords(first, first.getStart()-size, first.getStart());
        addFeature(Feature::INTERGENIC, coords);
    }
}

/** 
 * Set intergenic after the gene.
 */
void Gene::setAfterIntergenic(unsigned size) {
    assert(fFeatures.size() > 0);
    assert(fFeatures[fFeatures.size()-1].getType() != Feature::INTERGENIC);

    // adjust after size to prevent running of end of chromosome
    const Coords& last = fFeatures[fFeatures.size()-1];
    int seqSize = last.getSeqSize();
    int endPos = last.getEnd();
    if (endPos == seqSize) {
        size = 0;
    } else if (size >= (seqSize-endPos)) {
        size = (seqSize-endPos);
    }
    if (size > 0) {
        Coords coords(last, last.getEnd(), last.getEnd()+size);
        addFeature(Feature::INTERGENIC, coords);
    }
}

/**
 * Finish adding features, set the frame attributes and validates
 */
void Gene::completeFeatures() {
    assert(fFeatures.size() > 0);

    if (fSeq.size() > 0) {
        throw FatalError("Gene::completeFeatures() called after sequence has been set");
    }

    // Count and check features, set relative coordinates
    for (int i = 0; i < Feature::NUM_TYPES; i++) {
        fNumFeatures[i] = 0;
    }
    fNumRealFeatures = 0;
    Feature *prevFeat = NULL;
    int exonNum = -1;
    int intronNum = -1;
    for (int iFeat = 0; iFeat < fFeatures.size(); iFeat++) {
        Feature* feature = &fFeatures[iFeat];
        if ((prevFeat != NULL)
            && (prevFeat->getEnd() != feature->getStart())) {
            throw FatalError("gene features are discontiguous: " + fName);
        }
        if (feature->getType() != Feature::INTERGENIC) {
            fNumRealFeatures++;
        }

        // count biological exon/intron 
        if (feature->getBaseType() == Feature::EXON) {
            if ((prevFeat == NULL)
                || (prevFeat->getBaseType() != Feature::EXON)) {
                exonNum++;
                // count biological exons as well.
                fNumFeatures[Feature::typeToIdx(Feature::EXON)]++;
            }
            feature->fBaseTypeIdx = exonNum;
        } else if (feature->getBaseType() == Feature::INTRON) {
            if ((prevFeat == NULL)
                || (prevFeat->getBaseType() != Feature::INTRON)) {
                intronNum++;
            }
            feature->fBaseTypeIdx = intronNum;
        }
        fNumFeatures[Feature::typeToIdx(feature->getType())]++;
        prevFeat = feature;

        // sainty checks
        assert(feature->getName() == fFeatures[0].getName());
        assert(feature->getStrand() == fFeatures[0].getStrand());
        assert(feature->getSeqSize() == fFeatures[0].getSeqSize());
    }
    
    if (fFeatures[0].getType() == Feature::INTERGENIC) {
        fFirstRealFeatureIdx = 1;
    } else {
        fFirstRealFeatureIdx = 0;
    }

    // set links
    prevFeat = &(fFeatures[0]);
    prevFeat->fPrev = NULL;

    for (int iFeat = 1; iFeat < fFeatures.size(); iFeat++) {
        Feature* feature = &fFeatures[iFeat];
        prevFeat->fNext = feature;
        feature->fPrev = prevFeat;
        prevFeat = feature;
    }
    prevFeat->fNext = NULL;

    // find CDS
    int cdsStartIdx = -1;
    int cdsEndIdx = -1;
    for (int iFeat = 0; iFeat < fFeatures.size(); iFeat++) {
        if (fFeatures[iFeat].getType() == Feature::CDS) {
            if (cdsStartIdx < 0) {
                cdsStartIdx = iFeat;
            }
            cdsEndIdx = iFeat;
        }
    }

    // set gene-relative feature coordinates
    const Feature* first = getRealFeature(0);
    const Feature* last = getRealFeature(getNumRealFeatures()-1);
    fCoords = Coords(first->getName(), Coords::STRAND,
                     first->getStrand(), first->getStart(),
                     last->getEnd(), first->getSeqSize());
    first = getFeature(0);
    last = getFeature(getNumFeatures()-1);
    fSeqCoords = Coords(first->getName(), Coords::STRAND,
                        first->getStrand(), first->getStart(),
                        last->getEnd(), first->getSeqSize());

    for (int iFeat = 0; iFeat < fFeatures.size(); iFeat++) {
        Feature* feature = &fFeatures[iFeat];
        int relStart = feature->getStart()-fSeqCoords.getStart();
        feature->fGeneCoords
            = Coords(getName(), Coords::GENOMIC, Coords::NO_STRAND,
                     relStart, relStart+feature->getLength(),
                     fSeqCoords.getLength());
    }
    if (cdsStartIdx >= 0) {
        fCdsCoords = Coords(fCoords, 
                            fFeatures[cdsStartIdx].getStart(), 
                            fFeatures[cdsEndIdx].getEnd());
    } else {
        // get coord sys same, but NULL
        fCdsCoords = Coords(fCoords, 0, 0);
    }

    // set exon types
    int numExons = fNumFeatures[Feature::typeToIdx(Feature::EXON)];
    assert(numExons > 0);
    for (int iFeat = 0; iFeat < fFeatures.size(); iFeat++) {
        Feature* feature = &fFeatures[iFeat];
        if (feature->getBaseType() == Feature::EXON) {
            if (numExons == 1) {
                feature->fExonType = Feature::SINGLE_EXON;
            } else if (feature->getBaseTypeIdx() == 0) {
                feature->fExonType = Feature::INITIAL_EXON;
            } else if (feature->getBaseTypeIdx() == numExons-1) {
                feature->fExonType = Feature::FINAL_EXON;
            } else {
                feature->fExonType = Feature::INTERNAL_EXON;
            }
        }
    }

    // sanity check the whole gene
    assert(fNumFeatures[Feature::typeToIdx(Feature::INTERGENIC)] <= 2);

    // check for weird feature combinations.  This should have been
    // validated by the reader, hence these are just asserts.
    bool seenCds = false;
    bool seenUtr3 = false;
    for (int iFeat = 0; iFeat < fFeatures.size(); iFeat++) {
        Feature* feature = &fFeatures[iFeat];
        switch (feature->getType()) {
        case Gene::Feature::UTR5:
            assert(!seenCds);
            assert(!seenUtr3);
            break;
        case Gene::Feature::CDS:
            assert(!seenUtr3);
            seenCds = true;
            break;
        case Gene::Feature::UTR3:
            assert(seenCds);
            seenUtr3 = false;
            break;
        case Gene::Feature::INTRON:
        case Gene::Feature::EXON:
        case Gene::Feature::INTERGENIC:
            break;
        default:
            assert(false);
            break;
        }
    }
    // FIXME: create a validate method that throws exceptions.
}

/** 
 * Print a feature and optionally the associated sequence
 */
void Gene::dump(ostream& out,
                int indent,
                unsigned flags,
                const Feature* feature) const {
    string indentStr = StringOps::replicate(indent);
    string desc = feature->getTypeName();
    if (feature->getBaseTypeIdx() >= 0) {
        desc += " " + Convert::toString(feature->getBaseTypeIdx());
    }
    desc += ":";
    Coords genomic(*feature, Coords::GENOMIC);
    out << indentStr
        << StringOps::padRight(desc, 10)
        << *feature << " (" << genomic << ")";
    if (feature->getType() == Feature::CDS) {
        out << " [" << feature->getStartFrame() << "-"  << feature->getEndFrame() << "]";
    }
    out << endl;
    if ((flags & DUMP_SEQ) && (feature->getSeq().size() > 0)) {
        out << indentStr << "  " << feature->getSeq() << endl;
    }
}

/** 
 * Print the gene for debugging purposes
 */
void Gene::dump(ostream& out,
                int indent,
                unsigned flags) const {
    if (flags & DUMP_SEQ) {
        flags |= DUMP_FEATURES;
    }
    out << StringOps::replicate(indent)
        << "Gene: "  << fName << ": " << fCoords
        << " (" << Coords(fCoords, Coords::GENOMIC) << ") ";
    
    if (fSeqCoords != fCoords) {
        out << " seq=" << fSeqCoords;
    }
    out << endl;

    if (flags & DUMP_FEATURES) {
        for (int iFeat = 0; iFeat < fFeatures.size(); iFeat++) {
            dump(out, indent+4, flags, &fFeatures[iFeat]);
        }
    }
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
