#include "GeneChecker.h"
#include "CodonIterator.h"
#include "VerboseOption.h"
#include "StringOps.h"
#include "FatalError.h"

// FIXME: recording gene errors in both the gene as bits and here is
// really messy.  Make an object to hold error desc.

// FIXME split into UTR and CDS splice site problems.

/* Headers for details file */
const string GeneChecker::DETAILS_HDR1 = "acc\tproblem\tinfo\tchr\tchrStart\tchrEnd";

static VerboseOption sTrace(
    "gene-check",
    "trace validation of genes"
);

static VerboseOption sDumpGenes(
    "dump-genes",
    "dump gene and sequence for all genes"
);

static VerboseOption sDumpInvalid(
    "dump-invalid-genes",
    "dump gene and sequence for genes that fail the valility tests"
);

static const char TAB = '\t';

/* symbols for problems */
static const string BAD_FRAME_SYM = "badFrame";
static const string NO_START_CODON_SYM = "noStart";
static const string NO_STOP_CODON_SYM = "noStop";
static const string IN_FRAME_STOP_CODON_SYM = "orfStop";
static const string CDS_GAP_SYM = "cdsGap";
static const string UTR_GAP_SYM = "utrGap";
static const string CDS_NONCANON_SPLICE_SYM = "nonCanonCdsSplice";
static const string UTR_NONCANON_SPLICE_SYM = "nonCanonUtrSplice";
static const string CDS_UNKNOWN_SPLICE_SYM = "unknownCdsSplice";
static const string UTR_UNKNOWN_SPLICE_SYM = "unknownUtrSplice";
static const string NO_CDS_SYM = "noCds";
static const string FRAME_MISMATCH_SYM = "frameMismatch";
static const string FRAME_DISCONTIG_SYM = "frameDiscontig";
static const string NMD_SYM = "nmd";

/** 
 * Constructor
 */
GeneChecker::GeneChecker(unsigned options,
                         ostream* details):
    fOptions(options),
    fMinIntronSize(DEFAULT_MIN_INTRON),
    fDetails(details) {
    init(NULL);
}

/**
 * Initialize for a new gene.
 */
void GeneChecker::init(Gene* gene) {
    fGene = gene;
    fProblems = 0;
    fNumInFrameStop = 0;
    fNumIntrons = 0;
    fNumCdsGaps = 0;
    fNumUtrGaps = 0;
    fNumNonCanonicalUtrSplices = 0;
    fNumNonCanonicalCdsSplices = 0;
    fNumUnknownUtrSplices = 0;
    fNumUnknownCdsSplices = 0;
    fMessages.resize(0);
}

/**
 * Return a symbolic string for a given problem.
 */
const string& GeneChecker::getProblemSym(unsigned probFlag) {
    switch (probFlag) {
    case BAD_FRAME: return BAD_FRAME_SYM;
    case NO_START_CODON: return NO_START_CODON_SYM;
    case NO_STOP_CODON: return NO_STOP_CODON_SYM;
    case IN_FRAME_STOP_CODON: return IN_FRAME_STOP_CODON_SYM;
    case CDS_GAP: return CDS_GAP_SYM;
    case UTR_GAP: return UTR_GAP_SYM;
    case CDS_NONCANON_SPLICE: return CDS_NONCANON_SPLICE_SYM;
    case UTR_NONCANON_SPLICE: return UTR_NONCANON_SPLICE_SYM;
    case CDS_UNKNOWN_SPLICE: return CDS_UNKNOWN_SPLICE_SYM;
    case UTR_UNKNOWN_SPLICE: return UTR_UNKNOWN_SPLICE_SYM;
    case NO_CDS: return NO_CDS_SYM;
    case FRAME_MISMATCH: return FRAME_MISMATCH_SYM;
    case FRAME_DISCONTIG: return FRAME_DISCONTIG_SYM;
    case NMD: return NMD_SYM;
    }
    throw FatalError("BUG: invalid flag passed GeneCheck::getProblemSym()");
    return NMD_SYM;  /* keep compiler happy */
}

/**
 * Verbose output.
 */
void GeneChecker::traceCheck() {
    static const int INDENT = 4;
    if (fProblems != 0) {
        if (sDumpInvalid.isOn() || sTrace.isOn()) {
            VerboseOption& verb
                = (sDumpInvalid.isOn() ? sDumpInvalid : sTrace);
            verb.outPrefix() << "Gene checks failed for: "
                             << fGene->getName() << endl;
            fGene->dump(verb.getOut(), INDENT, 0);
            for (int idx = 0; idx < fMessages.size(); idx++) {
                verb.getOut() << StringOps::replicate(INDENT+2)
                              << fMessages[idx] << endl;
            }
            if (sDumpInvalid.isOn()) {
                fGene->dump(sDumpInvalid.getOut(), INDENT, Gene::DUMP_SEQ);
            }
        } else if (sDumpGenes.isOn()) {
            sDumpGenes.outPrefix() << "gene checks failed for: " << fGene->getName() << endl;
            fGene->dump(sDumpGenes.getOut(), INDENT, Gene::DUMP_SEQ);
        }
    } else if (sDumpGenes.isOn() || sTrace.isOn()) {
        VerboseOption& verb
            = (sDumpGenes.isOn() ? sDumpGenes : sTrace);
        verb.outPrefix() << "gene ok: " << fGene->getName() << endl;
        if (sDumpGenes.isOn()) {
            fGene->dump(sDumpGenes.getOut(), INDENT, Gene::DUMP_SEQ);
        }
    }
}

/*
 * Print details
 */
void GeneChecker::prDetails(unsigned probFlag,
                            const Coords& pos,
                            const string& info) {
    if (fDetails != NULL) {
        Coords gpos = pos.toGenomic();
        *fDetails << fGene->getName()
                  << TAB << getProblemSym(probFlag)
                  << TAB << info
                  << TAB << gpos.getName()
                  << TAB << gpos.getStart() 
                  << TAB << gpos.getEnd()
                  << endl;
    }
}

/*
 * Get a string describing the position of a feature.
 */
string GeneChecker::getFeatureLocDesc(const Gene::Feature* feature,
                                      const Coords& pos) const {
    const Coords& usePos = pos.isNull() ? *feature : pos;
    string desc = usePos.toString();

    if (usePos.getStrand() == Coords::NEG_STRAND) {
        Coords genomicPos(usePos, Coords::GENOMIC);
        desc + " (" + genomicPos.toString() + ") ";
    }
    if ((feature->getBaseType() == Gene::Feature::INTRON)
        || (feature->getBaseType() == Gene::Feature::EXON)) {
        desc += " " + feature->getTypeName() + " "
            + Convert::toString(feature->getBaseTypeIdx());
        if (!pos.isNull()) {
            // offset in feature
            unsigned offset = pos.getStart() - feature->getStart();
            desc += " off " + Convert::toString(offset);
        }
    }
    return desc;
}

/*
 * check for having CDS annotation
 */
bool GeneChecker::checkCDS() {
    if (fGene->getNumFeatures(Gene::Feature::CDS) == 0) {
        fMessages.add("No CDS defined");
        fProblems |= NO_CDS;
        prDetails(NO_CDS, fGene->getCoords());
        return false;
    } else {
        return true;
    } 
}

/*
 * Check frame annotation of a CDS exon
 */
void GeneChecker::checkExonFrame(const Gene::Feature* cds,
                                 const Gene::Feature* prevCds,
                                 int iCdsBase) {
    // end frame is frame of start of next cds
    // generate frameDiscontig first, and only generate frameMismatch if there are no
    // frameDiscontin errors.

    if ((prevCds != NULL) && (cds->getStartFrame() != prevCds->getEndFrame())) {
        fProblems |= FRAME_DISCONTIG;
        if (fOptions & FRAME_DISCONTIG) {
            fMessages.add("frame annotation discontinuous with previous CDS exon at " + cds->toGenomic().toString());
            prDetails(FRAME_DISCONTIG, *cds);
        }
    }

    int iCdsBaseNext = iCdsBase+cds->getLength();
    if ((!(fProblems & FRAME_DISCONTIG))
        && ((cds->getStartFrame() != (iCdsBase % 3))
            || (cds->getEndFrame() != (iCdsBaseNext % 3)))) {
        fProblems |= FRAME_MISMATCH;
        if (fOptions & FRAME_MISMATCH) {
            fMessages.add("frame annotation doesn't match for CDS exon at " + cds->toGenomic().toString());
            prDetails(FRAME_MISMATCH, *cds);
        }
    }
}

/*
 * Check frame annotation
 */
void GeneChecker::checkFrame() {
    int iCdsBase = 0;
    const Gene::Feature* cds = fGene->getFirstFeature(Gene::Feature::CDS);
    const Gene::Feature* prevCds = NULL;
    while (cds != NULL) {
        checkExonFrame(cds, prevCds, iCdsBase);
        iCdsBase += cds->getLength();
        prevCds = cds;
        cds = cds->getNext(Gene::Feature::CDS);
    }
    if ((iCdsBase % 3) != 0) {
        fProblems |= BAD_FRAME;
        if (fOptions & BAD_FRAME) {
            fMessages.add("CDS doesn't end on a frame boundry");
            prDetails(BAD_FRAME, fGene->getCdsCoords());
        }
    }
}

/*
 * is the frame ok?  checkFrame must be called first
 */
bool GeneChecker::isFrameOk() {
    return (fProblems & (BAD_FRAME|FRAME_MISMATCH|FRAME_DISCONTIG)) == 0;
}

/*
 * Check the first codon of a gene.
 */
void GeneChecker::checkFirstCodon(const CodonIterator& codonIter) {
    const Codon& codon = codonIter.getCodon();
    if (!codon.isStart()) {
        fProblems |= NO_START_CODON;
        if (fOptions & NO_START_CODON) {
            fMessages.add("does not begin with a start codon");
            Coords loc = codonIter.getCodonRange();
            prDetails(NO_START_CODON, loc);
        }
    }    
}

/*
 * do work of checking stop/start codons and frame.
 * checkFrame() must have already been called
 */
void GeneChecker::checkCodons() {
    vector<CodonIterator> stopCodons;
    int lastCodonNum = -1;
    bool lastIsStop = false;

    // Scan for stop codons and check exon frames.
    CodonIterator codonIter(fGene);
    while (codonIter.nextCodon()) {
        const Codon& codon = codonIter.getCodon();
        if (codonIter.getCodonNum() == 0) {
            checkFirstCodon(codonIter);
        }
            
        if (codon.isStop()) {
            stopCodons.push_back(codonIter);
            lastIsStop = true;
        } else {
            lastIsStop = false;
        }
        lastCodonNum = codonIter.getCodonNum();
    }

    if (!lastIsStop) {
        fProblems |= NO_STOP_CODON;
        if (fOptions & NO_STOP_CODON) {
            fMessages.add("does not end in a stop codon");
            Coords loc = codonIter.getCodonRange();
            prDetails(NO_STOP_CODON, loc);
        }
    }

    // report inframe stop codons if frame is broken tend to get tons of
    // in-frame stop, so we don't report then as problem, just include the
    // count.
    fNumInFrameStop = stopCodons.size() - (lastIsStop ? 1 : 0);
    if ((fNumInFrameStop > 0) && isFrameOk()) {
        assert(codonIter.isFrameOk());
        fProblems |= IN_FRAME_STOP_CODON;
        if (fOptions & IN_FRAME_STOP_CODON) {
            for (unsigned idx = 0; idx < fNumInFrameStop; idx++) {
                string desc = getFeatureLocDesc(stopCodons[idx].getCodonStartCds(),
                                               stopCodons[idx].getStartCoords());
                fMessages.add("stop codon ("+ stopCodons[idx].getCodon() + ") in CDS at " + desc);
                Coords loc = stopCodons[idx].getCodonRange();
                prDetails(IN_FRAME_STOP_CODON, loc, stopCodons[idx].getCodon());
            }
        }
    }
}

/**
 * Is a splice on of the three most common known one?
 */
bool GeneChecker::isKnownSplice(const Gene::Feature* intron) {
    string startSplice = intron->getStartSplice();
    string endSplice = intron->getEndSplice();

    // known: GT..AG, GC..AG, AT..AC
    return ((startSplice == "GT") && (endSplice == "AG"))
        || ((startSplice == "GC") && (endSplice == "AG"))
        || ((startSplice == "AT") && (endSplice == "AC"));
}

/**
 * Is a splice canonical?
 */
bool GeneChecker::isCanonicalSplice(const Gene::Feature* intron) {
    string startSplice = intron->getStartSplice();
    string endSplice = intron->getEndSplice();

    // canonical: GT..AG
    return ((startSplice == "GT") && (endSplice == "AG"));
}


/* check if an intron should be considered an intron or a gap */
bool GeneChecker::isGap(const Gene::Feature* intron) {
    return (intron->getLength() < fMinIntronSize);
}

/* Record a small gap */
void GeneChecker::recordGap(const Gene::Feature* gap) {
    if (gap->overlaps(fGene->getCdsCoords())) {
        fNumCdsGaps++;
        fProblems |= CDS_GAP;
        fMessages.add("has small CDS gap of " + Convert::toString(gap->getLength()));
        prDetails(CDS_GAP, *gap);
    } else {
        fNumUtrGaps++;
        fProblems |= UTR_GAP;
        fMessages.add("has small UTR gap of " + Convert::toString(gap->getLength()));
        prDetails(UTR_GAP, *gap);
    }
}

/*
 * Record error details about an intron splice
 */
void GeneChecker::recordSpliceErrors(const Gene::Feature* intron,
                                     unsigned probFlag) {
    string intronType = (probFlag & (CDS_NONCANON_SPLICE|CDS_UNKNOWN_SPLICE))
        ? "cds" : "utr";
    string probDesc = (probFlag & (CDS_NONCANON_SPLICE|UTR_NONCANON_SPLICE))
        ? "non-canonical" : "unknown";
    string loc = getFeatureLocDesc(intron);
    fMessages.add(intronType + " intron splice " + Convert::toString(intron->getBaseTypeIdx())
                  + " is " + probDesc + ": " + intron->getStartSplice() + ".."
                  + intron->getEndSplice() + " at " + loc);
    prDetails(probFlag, *intron, (intron->getStartSplice() + ".." + intron->getEndSplice()));
}

/**
 * Validate an intron; must have first checked that it doesn't get classified
 * as a gap.
 */
void GeneChecker::checkIntron(const Gene::Feature* intron) {
    bool isCds = intron->overlaps(fGene->getCdsCoords());
    bool recorded = false;   // don't want record details twice
    if (!isKnownSplice(intron)) {
        unsigned probFlag = 0;
        if (isCds) {
            fNumUnknownCdsSplices++;
            probFlag = CDS_UNKNOWN_SPLICE;
        } else {
            fNumUnknownUtrSplices++;
            probFlag = UTR_UNKNOWN_SPLICE;
        }
        if (probFlag & fOptions) {
            recordSpliceErrors(intron,  probFlag);
            recorded = true;
        }
        fProblems |= probFlag;
    }
    if (!isCanonicalSplice(intron)) {
        unsigned probFlag = 0;
        if (isCds) {
            fNumNonCanonicalCdsSplices++;
            probFlag = CDS_NONCANON_SPLICE;
        } else {
            fNumNonCanonicalUtrSplices++;
            probFlag = UTR_NONCANON_SPLICE;
        }
        if ((probFlag & fOptions) && !recorded) {
            recordSpliceErrors(intron,  probFlag);
        }
        fProblems |= probFlag;
    }
    fNumIntrons++;
}

/*
 * Check for non-canonical intron splicing and gaps
 */
void GeneChecker::checkIntrons() {
    const Gene::Feature* intron = fGene->getFirstFeature(Gene::Feature::INTRON);
    while (intron != NULL) {
        if (isGap(intron)) {
            recordGap(intron);
        } else {
            checkIntron(intron);
        }
        intron = intron->getNext(Gene::Feature::INTRON);
    }
}

/*
 * Find the mRNA distance from the stop codon to the last splice (this
 * excludes introns).  Returns -1 if no last splice.
 */
int GeneChecker::distToLastSplice() {
    int dist = 0;
    // first 3'UTR
    const Gene::Feature* utr3 = fGene->getFirstFeature(Gene::Feature::UTR3);
    if (utr3->getNext(Gene::Feature::UTR3) == NULL) {
        return -1;
    }
    // count until there is one 3'UTR left
    while (utr3->getNext(Gene::Feature::UTR3) != NULL) {
        dist += utr3->getLength();
        utr3 = utr3->getNext(Gene::Feature::UTR3);
    }
    return dist;
}

/*
 * Check for NMD mRNA: stop >= 55 from last splice
 */
void GeneChecker::checkNMD() {
    // We must have stop and 3'UTR annotation to be able to check for NMD
    if (((fProblems & NO_STOP_CODON) == 0)
        && (fGene->getNumFeatures(Gene::Feature::UTR3) > 0)) {
        if (distToLastSplice() > 55) {
            fProblems |= NMD;
        }
    }
}

/*
 * Check for a start codon and stop codons in the exons of a gene,
 * given the constraints.
 */
bool GeneChecker::codonCheck(Gene* gene) {
    init(gene);
    if (checkCDS()) {
        // checkFrame must be called first
        checkFrame();
        checkCodons();
    }
    traceCheck();
    return (getErrors() == 0);
}

/** 
 * Check that an annontation and a sequence look same
 */
bool GeneChecker::fullCheck(Gene* gene) {
    init(gene);
    if (checkCDS()) {
        // checkFrame must be called first
        checkFrame();
        checkCodons();
    }
    checkIntrons();
    checkNMD();
    traceCheck();
    return (getErrors() == 0);
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */
