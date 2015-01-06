#ifndef GENE_H
#define GENE_H

using namespace std;
#include <typeinfo>
#include <string>
#include <vector>
#include "Coords.h"

/**
 * Object used to record the annotation of a gene.
 */
class Gene {
  public:
    /** Dump flags */
    static const unsigned DUMP_FEATURES = 0x01;
    static const unsigned DUMP_SEQ      = 0x02;

    /** CDS status type, kept for start and end */
    typedef enum CDSStatus {
        cdsNone,
        cdsUnknown,
        cdsIncomplete,
        cdsComplete
    } CDSStatus;

    /**
     * A feature in the gene
     */
    class Feature: public Coords {
        friend class Gene;

        public:
        /** 
         * Constants for gene feature types, can be used as a bitset
         * as well as scalar ids.
         */
        typedef enum {
            UTR5       = 0x01,
            CDS        = 0x02,
            INTRON     = 0x04,
            UTR3       = 0x08,
            EXON       = 0x10,     // if CDS annotation is not available
            INTERGENIC = 0x20,
        } Type;
        static const int NUM_TYPES = 6;
        
        /* Set of types that select exon features */
        static const unsigned EXON_MASK = (UTR5|CDS|UTR3|EXON);

        /*
         * Classification of exons.
         */
        typedef enum {
            NOT_EXON        = 0x00,
            SINGLE_EXON     = 0x01,
            INITIAL_EXON    = 0x02,
            INTERNAL_EXON   = 0x04,
            FINAL_EXON      = 0x08,
        } ExonType;
        static const int NUM_EXON_TYPES = 5;


        private:
        /* State */
        Gene* fGene;
        Type fType;
        ExonType fExonType;

        /* list of features */
        Feature* fPrev;
        Feature* fNext;

        /* index of biological exon or intron */
        int fBaseTypeIdx;
        
        /** Frame of first base and frame after last base, or passive frame of
         * intron */
        int fStartFrame;
        int fEndFrame;

        /** coordinates with in the gene annotation */
        Coords fGeneCoords;

        /* Subsequence associated with feature, built in a lazy manner */
        string fSeq;

        /** internal methods */
        void buildSeq();

        public:
        /* Constructor */
        Feature(Gene* gene,
                const Coords& coords,
                Type type):
            Coords(coords),
            fGene(gene),
            fType(type),
            fExonType(NOT_EXON),
            fPrev(NULL),
            fNext(NULL),
            fBaseTypeIdx(-1),
            fStartFrame(-1),
            fEndFrame(-1) {
            assert(!coords.isNull());
        }

        /* Get the Gene */
        Gene* getGene() const {
            return fGene;
        }
        
        /* Get the type */
        Type getType() const {
            return fType;
        }

        /* Convert a feature type to an index */
        static int typeToIdx(Type type) {
            switch (type) {
            case UTR5:
                return 0;
            case CDS:
                return 1;
            case INTRON:
                return 2;
            case UTR3:
                return 3;
            case EXON:
                return 4;
            case INTERGENIC:
                return 5;
            default:
                assert(false);
                return 0;
            }
        }

        /* Get the type as a string */
        const string& getTypeName() const;

        /* Get the base type; UTR and CDS report as EXON */
        Type getBaseType() const {
            if ((fType == UTR5) || (fType == CDS) || (fType == UTR3)) {
                return EXON;
            } else {
                return fType;
            }
        }

        /* Get the biological exon or intron index */
        int getBaseTypeIdx() const {
            return fBaseTypeIdx;
        }

        /* Get the exon type */
        ExonType getExonType() const {
            return fExonType;
        }

        /* Convert a exon type to an index */
        static int exonTypeToIdx(ExonType exonType) {
            switch (exonType) {
            case NOT_EXON:
                return 0;
            case SINGLE_EXON:
                return 1;
            case INITIAL_EXON:
                return 2;
            case INTERNAL_EXON:
                return 3;
            case FINAL_EXON:
                return 4;
            default:
                assert(false);
                return 0;
            }
        }

        /* Get the exon type name */
        const string& getExonTypeName() const;

        /* Get the previous feature in the gene */
        const Feature* getPrev() const {
            return fPrev;
        }
        
        /* Get the next feature in the gene */
        const Feature* getNext() const {
            return fNext;
        }
        
        /* Get the previous feature matching any type in the set */
        const Feature* getPrev(unsigned typeSet) const {
            const Feature* feat = fPrev;
            while ((feat != NULL) && (!(feat->getType() & typeSet))) {
                feat = feat->fPrev;
            }
            return feat;
        }
        
        /* Get the next feature matching any type in the set */
        const Feature* getNext(unsigned typeSet) const {
            const Feature* feat = fNext;
            while ((feat != NULL) && (!(feat->getType() & typeSet))) {
                feat = feat->fNext;
            }
            return feat;
        }
        
        /* get start frame */
        int getStartFrame() const {
            return fStartFrame;
        }
        /* get end frame */
        int getEndFrame() const {
            return fEndFrame;
        }

        /* Get the coordinates within the gene */
        const Coords& getGeneCoords() const {
            return fGeneCoords;
        }

        /* Get the offset in the sequence associated with the gene */
        int getSeqOff() const {
            return getStart()-fGene->getSeqCoords().getStart();
        }

        /* Get the sequence associated with this feature */
        const string& getSeq() const {
            if ((fSeq.size() == 0) && (fGene->getSeq().size() > 0)) {
                const_cast<Feature*>(this)->buildSeq();
            }
            return fSeq;
        }

        /* Get start (5') splice site (if this is an intron), convert to
         * upper case. */
        string getStartSplice() const;

        /* Get end (3') splice site (if this is an intron), convert to
         * upper case. */
        string getEndSplice() const;
    };

  private:
    /** Attributes of the gene. */
    string fName;

    /** Range, excluding any intergenic */
    Coords fCoords;

    /* CDS coordinates */
    Coords fCdsCoords;

    /* status of CDS */
    CDSStatus fCdsStartStat;
    CDSStatus fCdsEndStat;

    /** Range, including any intergenic */
    Coords fSeqCoords;

    /** sorted vector of features */
    vector<Feature> fFeatures;

    /** features excluding intergeric */
    unsigned fFirstRealFeatureIdx;
    unsigned fNumRealFeatures;

    /** Optional source of gene definition */
    string fSource;

    /** Error flags */
    unsigned fErrorFlags;

    /** Sequence associated with the gene */
    string fSeq;

    /** 
     * Counts, by feature type.  EXON counts are included even if there
     * is CDS annotation.
     */
    unsigned fNumFeatures[Feature::NUM_TYPES];

    /** Internal methods */
    void dump(ostream& out,
              int indent,
              unsigned flags,
              const Feature* feature) const;

    public:
    /** Constructor */
    Gene(const string& name);

    /** Add a feature */
    void addFeature(Feature::Type type,
                    const Coords& coords,
                    int frame = -1);
    
    /** Set preceeding intergenic. */
    void setBeforeIntergenic(unsigned size);

    /** Set intergenic after the gene. */
    void setAfterIntergenic(unsigned size);

    /** Finish adding features, set the frame attributes and validates. */
    void completeFeatures();

    /** Get the name */
    const string& getName() const {
        return fName;
    }

    /** Get the coordinates the annotation (excluding intergenic) */
    const Coords& getCoords() const {
        return fCoords;
    }

    /** Get the coords of the whole anotation (including intergenic) */
    const Coords& getSeqCoords() const {
        return fSeqCoords;
    }

    /** Get the coordinates the CDS */
    const Coords& getCdsCoords() const {
        return fCdsCoords;
    }
    
    /** Get CDS start status */
    CDSStatus getCdsStartStat() const {
        return fCdsStartStat;
    }

    /** Get CDS end status */
    CDSStatus getCdsEndStat() const {
        return fCdsEndStat;
    }

    /** Set CDS status fields */
    void setCdsStat(CDSStatus startStat,
                    CDSStatus endStat) {
        fCdsStartStat = startStat;
        fCdsEndStat = endStat;
    }

    /* 
     * Get the the number of features of a specific type.  EXON counts will
     * reflect the biological exons even if CDS annotation is available and
     * there are no features of type `EXON'.
     */
    unsigned getNumFeatures(Feature::Type type) const {
        return fNumFeatures[Feature::typeToIdx(type)];
    }
    
    /** Get the number of features. */
    unsigned getNumFeatures() const {
        return fFeatures.size();
    }

    /** Get a feature */
    const Feature* getFeature(int idx) const {
        return &(fFeatures[idx]);
    }

    /** Get the number of real features (not intergenic). */
    unsigned getNumRealFeatures() const {
        return fNumRealFeatures;
    }

    /** Get a real feature */
    const Feature* getRealFeature(int idx) const {
        return &(fFeatures[fFirstRealFeatureIdx+idx]);
    }

    /* Get the first feature */
    const Feature* getFirstFeature() const {
        return &(fFeatures[0]);
    }

    /* Get the first feature matching one of the types in the set */
    const Feature* getFirstFeature(unsigned typeSet) const {
        const Feature* feat = &(fFeatures[0]);
        while ((feat != NULL) && (!(feat->getType() & typeSet))) {
            feat = feat->getNext();
        }
        return feat;
    }

    /** Set optional source of gene definition */
    void setSource(const string& source) {
        fSource = source;
    }

    /** get optional source of gene definition */
    const string& getSource() const {
        return fSource;
    }

    /** Set the sequence for the gene */
    void setSeq(const string& seq) {
        assert(seq.size() == fSeqCoords.getLength());
        fSeq = seq;
    }

    /** Get the sequence for the gene, or empty if not set */
    const string& getSeq() const {
        return fSeq;
    }

    /** Print the gene for debugging purposes */
    void dump(ostream& out,
              int indent,
              unsigned flags = DUMP_FEATURES) const;
};
#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
