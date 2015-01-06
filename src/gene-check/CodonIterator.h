#ifndef CODON_ITERATOR_H
#define CODON_ITERATOR_H

#include "FatalError.h"
#include "Codon.h"
#include "Gene.h"

/**
 * Iterator over codons in a seq.  Idea and some code stolen from Krish..
 */
class CodonIterator {
  private:
    const Gene* fGene;
    const string* fSeq;

    /* Current CDS exon and offset info */
    const Gene::Feature* fCurCds;
    int fCurCdsOff;
    int fCurCdsIdx;
    int fCurCdsSize;

    /** 
     * Current codon, including starting and ending exon and offset in the
     * the exon
     */
    int fCodonNum;
    const Gene::Feature* fCodonStartExon;
    int fCodonStartIdx;
    const Gene::Feature* fCodonMidExon;
    int fCodonMidIdx;
    const Gene::Feature* fCodonEndExon;
    int fCodonEndIdx;
    Codon fCodon;
    bool fFrameOk;

    /** Set state to the state of the gene */
    void reset() {
        fCurCds = NULL;
        fCurCdsOff = -1;
        fCurCdsIdx = -1;
        fCurCdsSize = -1;
        fCodonNum = -1;
        fCodonStartExon = NULL;
        fCodonStartIdx = 0;
        fCodonMidExon = NULL;
        fCodonMidIdx = 0;
        fCodonEndExon = NULL;
        fCodonEndIdx = 0;
        fFrameOk = true;
    }

  public:
    /** 
     * Constructor
     */
    CodonIterator(const Gene* gene):
        fGene(gene),
        fSeq(&gene->getSeq()) {
        reset();
    }

    /** the gene we are associated with */
    const Gene* getGene() const {
        return fGene;

    }

    private:
    /** Advance to the next exon */
    bool nextExon() {
        if (fCurCds == NULL) {
            fCurCds = fGene->getFirstFeature(Gene::Feature::CDS);
        } else {
            fCurCds = fCurCds->getNext(Gene::Feature::CDS);
        }
        if (fCurCds == NULL) {
            return false;
        } else {
            fCurCdsOff = fCurCds->getSeqOff();
            fCurCdsIdx = 0;
            fCurCdsSize = fCurCds->getLength();
            return true;
        }
    }

  private:
    /* finish off last codon */
    void lastCodon(int iCodon) {
        // if frame not ok, clear remaining codon position info
        switch (iCodon) {
        case 0:
            fFrameOk = true;
            break;
        case 1:
            fCodonMidExon = NULL;
            fCodonMidIdx = -1;
        case 2:
            fCodonEndExon = NULL;
            fCodonEndIdx = -1;
            fFrameOk = false;
            break;
        }
    }

  public:
    /** 
     * Advance to the next codon.  Will return false when there
     * are no more complete codons.  Check isFrameOk() to determine
     * if it ended on a frame boundry.
     */
    bool nextCodon() {
        for (int iCodon = 0; iCodon < 3; iCodon++) {
            fCurCdsIdx++;
            if (fCurCdsIdx >= fCurCdsSize) {
                if (!nextExon()) {
                    lastCodon(iCodon);
                    return false; // no more
                }
            }
            if (iCodon == 0) {
                // record start of codon
                fCodonNum++;
                fCodonStartExon = fCurCds;
                fCodonStartIdx = fCurCdsIdx;
            } else if (iCodon == 1) {
                fCodonMidExon = fCurCds;
                fCodonMidIdx = fCurCdsIdx;
            } else {
                fCodonEndExon = fCurCds;
                fCodonEndIdx = fCurCdsIdx;
            }
            fCodon[iCodon] = (*fSeq)[fCurCdsOff+fCurCdsIdx];
        }
        return true;
    }

    /** Get current codon location */
    int getCodonNum() const {
        return fCodonNum;
    }
    const Gene::Feature* getCodonStartCds() const {
        return fCodonStartExon;
    }
    const Gene::Feature* getCodonMidCds() const {
        return fCodonMidExon;
    }
    const Gene::Feature* getCodonEndCds() const {
        return fCodonEndExon;
    }

    /* get position of first base of codon */
    Coords getStartCoords() const {
        unsigned start = fCodonStartExon->getStart()+fCodonStartIdx;
        return Coords(*fCodonStartExon, start, start+1);
    }

    /* get position of second base of codon, or NULL_COORD if incomplete */
    Coords getMidCoords() const {
        if (fCodonMidExon == NULL) {
            return Coords::NULL_COORD;
        } else {
            unsigned start = fCodonMidExon->getStart()+fCodonMidIdx;
            return Coords(*fCodonMidExon, start, start+1);
        }
    }

    /* get position of thrid base of codon, or NULL_COORD if incomplete */
    Coords getEndCoords() const {
        if (fCodonEndExon == NULL) {
            return Coords::NULL_COORD;
        } else {
            unsigned start = fCodonEndExon->getStart()+fCodonEndIdx;
            return Coords(*fCodonEndExon, start, start+1);
        }
    }

    /* Get entire range covered by codon, even if spliced, also handle
     * partial last exon. */
    Coords getCodonRange() const {
        unsigned start = fCodonStartExon->getStart()+fCodonStartIdx;
        unsigned end;
        if (fCodonEndExon != NULL) {
            end = fCodonEndExon->getStart()+fCodonEndIdx;  // complete
        } else if (fCodonMidExon != NULL) {
            end = fCodonMidExon->getStart()+fCodonMidIdx;
        } else {
            end = start;
        }
        return Coords(*fCodonStartExon, start, end+1);
    }

    /** Get the codon */
    const Codon& getCodon() const {
        return fCodon;
    }

    /** Check if we ended on a frame boundry */
    bool isFrameOk() const {
        return fFrameOk;
    }
};
#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */
