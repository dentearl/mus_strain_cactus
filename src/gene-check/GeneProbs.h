#ifndef GENE_PROBS_H
#define GENE_PROBS_H

using namespace std;
#include <typeinfo>
#include <string>
#include <vector>
#include "Coords.h"

/**
 * Object used to record gene problems detected by GeneCheck
 */
class GeneProbs: public RefCounted {
  public:
    /** Bit set of problem types */
    static const unsigned FRAME_ERR           = 0x001;  // CDS not multiple of 3
    static const unsigned NO_START_CODON      = 0x002;
    static const unsigned NO_STOP_CODON       = 0x004;
    static const unsigned IN_FRAME_STOP_CODON = 0x008;
    static const unsigned SMALL_GAP           = 0x010;
    static const unsigned CDS_SPLICE          = 0x020;  // non-canonical splice in CDS
    static const unsigned UTR_SPLICE          = 0x040;  // non-canonical splice in UTR
    static const unsigned NO_CDS              = 0x080;
    static const unsigned LOST_FRAME          = 0x100;  // have CDS, but frame is lost

    /* Class describing a single problem */
    class Prob {
        private:
        unsigned fType;  /* problem type */
        string fMsg;     /* description */
        Coords fCoords   /* problem coordiates */
        public:
        /* constructor */
        Prob(unsigned type,
             const string msg):
            fType(type),  
            fMsg(msg) {
        }
        /* accessors */
        unsigned getType() const {
            return fType;
        }
        const string& getMsg() const {
            return fmsg;
        }
    };

  private:
    Gene* fGene;
    unsigned fTypes;     /** Problem types */

    public:
    /** Constructor */
    GeneProbs(Gene* gene) {
        fGene = gene;
    }

    /** Get the gene */
    Gene* getGene() const {
        return fGene;
    }

    /** Is there any problem */
    bool hasProbs() const {
        return (fTypes != 0);
    }

    /** Get the problem types */
    unsigned getTypes() const {
        return fTypes;
    }
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
