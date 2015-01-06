#ifndef GENE_CHECKER_H
#define GENE_CHECKER_H

#include <string>
#include "Gene.h"
#include "StringVector.h"
class Gene;
class CodonIterator;

/**
 * Check that a gene annotation and sequence looks reasonable.
 */
class GeneChecker {
    public:
    /** Flags indication error conditions, which are also options indicating what
     * should be considered errors. */
    static const unsigned BAD_FRAME           = 0x0001;  // CDS not multiple of 3
    static const unsigned NO_START_CODON      = 0x0002;
    static const unsigned NO_STOP_CODON       = 0x0004;
    static const unsigned IN_FRAME_STOP_CODON = 0x0008;
    static const unsigned CDS_GAP             = 0x0010;
    static const unsigned UTR_GAP             = 0x0020;
    static const unsigned CDS_NONCANON_SPLICE = 0x0040;  // non-canonical splice in CDS
    static const unsigned UTR_NONCANON_SPLICE = 0x0080;  // non-canonical splice in UTR
    static const unsigned CDS_UNKNOWN_SPLICE  = 0x0100;  // unknown splice in CDS
    static const unsigned UTR_UNKNOWN_SPLICE  = 0x0200;  // unknown splice in UTR
    static const unsigned NO_CDS              = 0x0400;
    static const unsigned FRAME_MISMATCH      = 0x0800;  // exon annotation does match expected
    static const unsigned FRAME_DISCONTIG     = 0x1000;  // frame annotation not consistent between exons
    static const unsigned NMD                 = 0x2000;

    /* default doesn't include NMD, _NONCANON_SPLICE */
    static const unsigned DEFAULT_OPTIONS
        = BAD_FRAME|NO_START_CODON|NO_STOP_CODON|IN_FRAME_STOP_CODON|CDS_GAP|
        UTR_GAP|CDS_UNKNOWN_SPLICE|UTR_UNKNOWN_SPLICE|NO_CDS|FRAME_MISMATCH|
        FRAME_DISCONTIG;

    static const unsigned ALL_OPTIONS = 0xFFFF;
    static const int DEFAULT_MIN_INTRON = 20; /* min size for an intron */

    /* Headers for details file */
    static const string DETAILS_HDR1;

    private:
    /* flags controling validation */
    unsigned fOptions;

    /* minimum intron size, smaller are considered gaps */
    unsigned fMinIntronSize;

    /** Current gene */
    Gene* fGene;

    /** Problem on current gene, only consider an error when anded with
     * options., */
    unsigned fProblems;
    unsigned fNumInFrameStop;
    unsigned fNumIntrons;
    unsigned fNumCdsGaps;
    unsigned fNumUtrGaps;
    unsigned fNumNonCanonicalUtrSplices;
    unsigned fNumNonCanonicalCdsSplices;
    unsigned fNumUnknownUtrSplices;
    unsigned fNumUnknownCdsSplices;
    StringVector fMessages;

    /* write detailed table of errors to this file */
    ostream* fDetails;
    
    void init(Gene* gene);
    void traceCheck();
    void prDetails(unsigned probFlag, const Coords& pos, const string& info="");
    string getFeatureLocDesc(const Gene::Feature* feature,
                             const Coords& pos = Coords::NULL_COORD) const;
    bool checkCDS();
    void checkExonFrame(const Gene::Feature* cds,
                        const Gene::Feature* prevCds,
                        int iCdsBase);
    void checkFrame();
    void checkFirstCodon(const CodonIterator& codonIter);
    bool isFrameOk();
    void checkCodons();
    bool isGap(const Gene::Feature* intron);
    bool isKnownSplice(const Gene::Feature* gap);
    bool isCanonicalSplice(const Gene::Feature* gap);
    void recordGap(const Gene::Feature* intron);
    void recordSpliceErrors(const Gene::Feature* intron,
                            unsigned probFlag);
    void checkIntron(const Gene::Feature* intron);
    void checkIntrons();
    int distToLastSplice();
    void checkNMD();

    public:
    /** Constructor */
    GeneChecker(unsigned options = DEFAULT_OPTIONS,
                ostream* details = NULL);

    /* get/set minIntronSize */
    unsigned getMinIntronSize() const {
        return fMinIntronSize;
    }
    void getMinIntronSize(unsigned minIntronSize) {
       fMinIntronSize = minIntronSize;
    }

    /** Return a symbolic string for a given problem. */
    const string& getProblemSym(unsigned probFlag);

    /*
     * Check for a start codon and stop codons in the exons of a gene,
     * given the constraints.
     */
    bool codonCheck(Gene* gene);

    /* get the options */
    unsigned getOptions() const {
        return fOptions;
    }

    /** Check that an annontation and a sequence look same */
    bool fullCheck(Gene* gene);

    /* Get current problem bitset */
    unsigned getProblems() const {
        return fProblems;
    }

    /* Get current error bitset, these are problems recorded ANDed with
     * conditions that are considered errors */
    unsigned getErrors() const {
        return (fProblems & fOptions);
    }

    /* Get number of inframe stop codons */
    unsigned getNumInFrameStop() const {
        return fNumInFrameStop;
    }

    /* Get number of introns */
    unsigned getNumIntrons() const {
        return fNumIntrons;
    }

    /* Get number of small CDS gaps not counted as introns */
    unsigned getNumCdsGaps() const {
        return fNumCdsGaps;
    }

    /* Get number of small UTR gaps not counted as introns */
    unsigned getNumUtrGaps() const {
        return fNumUtrGaps;
    }

    /* Get number of non-canonical CDS introns */
    unsigned getNumNonCanonicalCdsIntrons() const {
        return fNumNonCanonicalCdsSplices;
    }

    /* Get number of non-canonical UTR introns */
    unsigned getNumNonCanonicalUtrIntrons() const {
        return fNumNonCanonicalUtrSplices;
    }

    /* Get number of unknown CDS introns */
    unsigned getNumUnknownCdsIntrons() const {
        return fNumUnknownCdsSplices;
    }

    /* Get number of unknown UTR introns */
    unsigned getNumUnknownUtrIntrons() const {
        return fNumUnknownUtrSplices;
    }
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
