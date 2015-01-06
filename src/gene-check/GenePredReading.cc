#include "GenePredReading.h"
#include "Gene.h"
#include "Genome.h"
#include "Convert.h"
#include <iostream>

extern "C" {
#include "common.h"
#include "genePred.h"
#include "genePredReader.h"
#include "linefile.h"
}

// FIXME: drop no_utr option.

/** 
 * Constructor.
 */
GenePredReading::GenePredReading(const string& tabFile,
                                 Genome* genome,
                                 unsigned options):
    fOptions(options),
    fIn(NULL),
    fGenome(genome),
    fIntergenic(0),
    fGenePred(NULL),
    fHaveCds(false),
    fCdsStartStat(Gene::cdsNone),
    fCdsEndStat(Gene::cdsNone) {

    fIn = genePredReaderFile((char*)tabFile.c_str(), NULL);
}

/* 
 * Destructor
 */
GenePredReading::~GenePredReading() {
    genePredFree(&fGenePred);
    genePredReaderFree(&fIn);
}

/**
 * Get the chromosome info for chrom.
 */
const Coords& GenePredReading::getChrom(const char *chrom) {
    if (fChrom.getName() != chrom) {
        fChrom = *fGenome->getChrom(string(chrom));
    }
    return fChrom;
}

/*
 * Create a feature.
 */
void GenePredReading::createFeature(const Coords& chrom,
                                   Gene* gene,
                                   char strand,
                                   int start,
                                   int end,
                                   Gene::Feature::Type type,
                                   int frame) {
    assert(chrom.getSystem() == Coords::GENOMIC);

    // compute coords in genomic then convert to strand
    Coords genomicCoords(chrom.getName(), Coords::GENOMIC,
                         strand, start, end, chrom.getSeqSize());
    Coords strandCoords(genomicCoords, Coords::STRAND);

    gene->addFeature(type, strandCoords, frame);
}


/*
 * Create features for an exon when CDS is available, splitting into UTR
 * and/or CDS.
 */
void GenePredReading::createExonWithCDS(const Coords& chrom,
                                       Gene* gene,
                                       int iExon) {
    char strand = fGenePred->strand[0];
    int exonNext = fGenePred->exonStarts[iExon];
    int exonEnd = fGenePred->exonEnds[iExon];

    // handle first UTR
    if (exonNext < fGenePred->cdsStart) {
        int utrEnd = (exonEnd < fGenePred->cdsStart)
            ? exonEnd : fGenePred->cdsStart;
        if (!(fOptions & NO_UTR)) {
            createFeature(chrom, gene, strand, exonNext, utrEnd,
                          ((strand == '+') ? Gene::Feature::UTR5
                           : Gene::Feature::UTR3));
        }
        exonNext = utrEnd;
    }
    assert(exonNext <= exonEnd);

    // handle CDS
    if ((exonNext >= fGenePred->cdsStart)
        && (exonNext < fGenePred->cdsEnd)) {
        int cdsEnd = (exonEnd < fGenePred->cdsEnd)
            ? exonEnd : fGenePred->cdsEnd;
        createFeature(chrom, gene, strand, exonNext, cdsEnd,
                      Gene::Feature::CDS, fExonFrames[iExon]);
        exonNext = cdsEnd;
    }
    assert(exonNext <= exonEnd);


    // handle last UTR
    if ((exonNext >= fGenePred->cdsEnd) && (fGenePred->cdsEnd < exonEnd)) {
        if (!(fOptions & NO_UTR)) {
            createFeature(chrom, gene, strand, exonNext, exonEnd,
                          ((strand == '+') ? Gene::Feature::UTR3
                           : Gene::Feature::UTR5));
        }
        exonNext = exonEnd;
    }
    assert(exonNext == exonEnd);
}

/*
 * Create features for an exon, splitting into UTR and/or CDS.
 */
void GenePredReading::createExon(const Coords& chrom,
                                Gene* gene,
                                int iExon) {
    if (fHaveCds) {
        createExonWithCDS(chrom, gene, iExon);
    } else {
        createFeature(chrom, gene, fGenePred->strand[0], 
                      fGenePred->exonStarts[iExon], fGenePred->exonEnds[iExon],
                      Gene::Feature::EXON);
    }
}

/*
 * Create an intron following the specified exon, if necessary
 */
void GenePredReading::createIntron(const Coords& chrom,
                                  Gene* gene,
                                  int iExon) {
    if (iExon < fGenePred->exonCount-1) {
        int start = fGenePred->exonEnds[iExon];
        int end = fGenePred->exonStarts[iExon+1];

        // if we are not extracting UTR, then we don't create introns if the 
        // is outside of the CDS.
        if ((!(fOptions & NO_UTR))
            || ((start > fGenePred->cdsStart) && (end < fGenePred->cdsEnd))) {
            createFeature(chrom, gene, fGenePred->strand[0],
                          start, end, Gene::Feature::INTRON);
        }
    }
}

/* convert genePred CDS status to Gene CDS status */
static Gene::CDSStatus cnvCdsStatus(enum cdsStatus gpStatus) {
    switch (gpStatus) {
    case cdsNone:
        return Gene::cdsNone;
    case cdsUnknown:
        return Gene::cdsUnknown;
    case cdsIncomplete:
        return Gene::cdsIncomplete;
    case cdsComplete:
        return Gene::cdsComplete;
    }
    assert(false);
    return Gene::cdsNone; /* invalid value */
}

/*
 * Compute CDS start/end status
 */
void GenePredReading::computeCdsStatus() {
    if (fGenePred->optFields & genePredCdsStatFld) {
        /* specified */
        if (fGenePred->strand[0] == '+') {
            fCdsStartStat = cnvCdsStatus(fGenePred->cdsStartStat);
            fCdsEndStat = cnvCdsStatus(fGenePred->cdsEndStat);
        } else {
            fCdsStartStat = cnvCdsStatus(fGenePred->cdsEndStat);
            fCdsEndStat = cnvCdsStatus(fGenePred->cdsStartStat);
        }
    } else {
        /* Assume the best */
        fCdsStartStat = Gene::cdsComplete;
        fCdsEndStat = Gene::cdsComplete;
    }
}

/*
 * Compute exon frame array.
 */
void GenePredReading::computeExonFrames() {
#if 0 // FIXME:
    // determine which direction to compute from.  We start
    // with 5' if possible.
    int dir;  // -1, 0, +1; 0 == skip
    if (fGenePred->strand[0] == '+') {
        dir = (fCdsStartStat == Gene::cdsComplete)
            ? 1 : ((fCdsEndStat == Gene::cdsComplete) ? -1 : 0);
    } else {
        dir = (fCdsStartStat == Gene::cdsComplete)
            ? -1 : ((fCdsEndStat == Gene::cdsComplete) ? 1 : 0);
    }
#endif
    int cdsOff = 0;
    int start, end;
    if (fGenePred->strand[0] == '+') {
        for (int iExon = 0; iExon < fGenePred->exonCount; iExon++) {
            if (genePredCdsExon(fGenePred, iExon, &start, &end)) {
                fExonFrames[iExon] = (cdsOff % 3);
                cdsOff += (end - start);
            }
        }
    } else {
        for (int iExon = fGenePred->exonCount-1; iExon >= 0; iExon--) {
            if (genePredCdsExon(fGenePred, iExon, &start, &end)) {
                fExonFrames[iExon] = (cdsOff % 3);
                cdsOff += (end - start);
            }
        }
    }
}

/*
 * Setup the exon frame array.
 */
void GenePredReading::setupExonFrames() {
    if (fGenePred->optFields & genePredExonFramesFld) {
        // given to us, easy
        for (int iExon = 0; iExon < fGenePred->exonCount; iExon++) {
            fExonFrames[iExon] = fGenePred->exonFrames[iExon];
        }
    } else {
        computeExonFrames();
    }
}

/*
 * Determine CDS and frame information and store in object.
 */
void GenePredReading::computeCDSInfo() {
    /* initialize */
    fHaveCds = (fGenePred->cdsStart < fGenePred->cdsEnd);
    fCdsStartStat = Gene::cdsNone;
    fCdsEndStat = Gene::cdsNone;
    fExonFrames.resize(fGenePred->exonCount);
    for (int iExon = 0; iExon < fGenePred->exonCount; iExon++) {
        fExonFrames[iExon] = -1;
    }
        
    if (fHaveCds) {
        computeCdsStatus();
        setupExonFrames();
    }
}

/** 
 * Convert a genePred to a Gene.  Pass chrom info as Coords ensures that the
 * name string is shared.
 */
Gene* GenePredReading::toGene(const Coords& chrom) {
    computeCDSInfo();

    Gene* gene = new Gene(string(fGenePred->name));

    gene->setCdsStat(fCdsStartStat, fCdsEndStat);
    
    for (unsigned iExon = 0; iExon < fGenePred->exonCount; iExon++) {
        createExon(chrom, gene, iExon);
        createIntron(chrom, gene, iExon);
    }
    if (fIntergenic > 0) {
        gene->setBeforeIntergenic(fIntergenic);
        gene->setAfterIntergenic(fIntergenic);
    }
    gene->completeFeatures();
    if (fOptions & READ_SEQS) {
        gene->setSeq(fGenome->read(gene->getSeqCoords()));
    }
    return gene;
}

/* 
 * Issue a warning about an invalid gene prediction.
 */
void GenePredReading::warn(const string& msg) {
    if (fOptions & VERBOSE_ERRORS) {
        cerr << "Warning: " << fGenePred->name
             << " " << fGenePred->chrom
             << ":" << fGenePred->txStart
             << "-" << fGenePred->txEnd << ": " << msg << endl;
    }
}

/* 
 * Validate a genePred structure for internal consistency.
 * Also check if CDS can be computed
 * FIXME: replace with call to genePredCheck function
 */
bool GenePredReading::check(const Coords& chrom) {
    unsigned numErrors = 0;
    unsigned iExon;
    bool haveCds = (fGenePred->cdsStart < fGenePred->cdsEnd);
    bool cdsStartInExon = false;
    bool cdsEndInExon = false;

    // strand
    string strand(fGenePred->strand);
    if (!((strand == "+") || (strand == "-"))) {
        warn("invalid strand: \"" + strand + "\"");
        numErrors++;
    }

    // in chromosome bounds
    if (fGenePred->txEnd > chrom.getLength()) {
        warn("txEnd " + Convert::toString(fGenePred->txEnd)
             + " >= chromSize " + Convert::toString(chrom.getLength()));
        numErrors++;
    }

    // internal consistency 
    if (fGenePred->txStart >= fGenePred->txEnd) {
        warn("txStart " + Convert::toString(fGenePred->txStart)
             + " >= txEnd " + Convert::toString(fGenePred->txEnd));
        numErrors++;
    }

    // no CDS annotation is indicated by cdsStart == cdsEnd, usually zero
    // or txEnd.
    if (fGenePred->cdsStart != fGenePred->cdsEnd) {
        if (fGenePred->cdsStart > fGenePred->cdsEnd) {
            warn("cdsStart " + Convert::toString(fGenePred->cdsStart)
                 + " > cdsEnd " + Convert::toString(fGenePred->cdsEnd));
            numErrors++;
        }
        if ((fGenePred->cdsStart < fGenePred->txStart)
            || (fGenePred->cdsStart > fGenePred->txEnd)) {
            warn("cdsStart " + Convert::toString(fGenePred->cdsStart)
                 + " not in tx bounds " + Convert::toString(fGenePred->txStart)
                 + "-" + Convert::toString(fGenePred->txEnd));
            numErrors++;
        }
        if ((fGenePred->cdsEnd < fGenePred->txStart) || (fGenePred->cdsEnd > fGenePred->txEnd)) {
            warn("cdsEnd " + Convert::toString(fGenePred->cdsEnd)
                 + " not in tx bounds " + Convert::toString(fGenePred->txStart)
                 + "-" + Convert::toString(fGenePred->txEnd));
            numErrors++;
        }
    }
    
    for (iExon = 0; iExon < fGenePred->exonCount; iExon++) {
        unsigned exonStart = fGenePred->exonStarts[iExon];
        unsigned exonEnd = fGenePred->exonEnds[iExon];
        if (exonStart >= exonEnd) {
            warn("exon " + Convert::toString(iExon)
                 + " start " + Convert::toString(exonStart)
                 + " >= end " + Convert::toString(exonEnd));
            numErrors++;
        }
        if (exonStart < fGenePred->txStart) {
            warn("exon " + Convert::toString(iExon)
                 + " start " + Convert::toString(exonStart)
                 + " < txStart " + Convert::toString(fGenePred->txStart));
            numErrors++;
        }
        if (exonEnd > fGenePred->txEnd) {
            warn("exon " + Convert::toString(iExon)
                 + " end " + Convert::toString(exonEnd)
                 + " > txEnd " + Convert::toString(fGenePred->txEnd));
            numErrors++;
        }
        if (iExon > 0) {
            unsigned prevExonEnd = fGenePred->exonEnds[iExon-1];
            if (exonStart < prevExonEnd) {
                warn("exon " + Convert::toString(iExon)
                     + " overlaps previous exon");
                numErrors++;
            }
        }
        if (haveCds) {
            if ((exonStart <= fGenePred->cdsStart) && (fGenePred->cdsStart < exonEnd)) {
                cdsStartInExon = true;
            }
            if ((exonStart <= fGenePred->cdsEnd-1) && (fGenePred->cdsEnd-1 < exonEnd)) {
                cdsEndInExon = true;
            }
        }
    }
    if (haveCds) {
        if (!cdsStartInExon) {
            warn("cdsStart not in an exon");
            numErrors++;
        }
        if (!cdsEndInExon) {
            warn("cdsEnd not in an exon");
            numErrors++;
        }
    }
    return (numErrors == 0);
}

/**
 * Read the next genePred, ignoring rows with errors. 
 */
Gene* GenePredReading::next() {
    Gene* gene = NULL;
    struct genePred *gp;
    
    // read until one validates; must keep genePred around, as well
    // allow access through class.
    while ((gene == NULL) && ((gp = genePredReaderNext(fIn)) != NULL)) {
        genePredFree(&fGenePred);
        fGenePred = gp;
        const Coords& chrom = getChrom(fGenePred->chrom);
        if (check(chrom)) {
            gene = toGene(chrom);
        }
    }
    return gene;
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
