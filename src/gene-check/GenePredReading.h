#ifndef GENE_PRED_READER_H
#define GENE_PRED_READER_H

using namespace std;
#include <typeinfo>
#include "Gene.h"
#include <stdio.h>
struct genePred;
struct genePredReader;
class Genome;
class Gene;

class GenePredReading {
    public:
    /* options */
    static const unsigned VERBOSE_ERRORS = 0x01;
    static const unsigned NO_UTR = 0x02;
    static const unsigned READ_SEQS = 0x04;  /* read sequences */

    private:
    unsigned fOptions;
    struct genePredReader* fIn;

    /* table with chrom sizes */
    Genome* fGenome;

    /* Amount of intergenic to add */
    unsigned fIntergenic;

    /* coord object for curent chromsome */
    Coords fChrom;

    /* Last genePred that was read, or NULL if none */
    struct genePred* fGenePred;

    /* is CDS defined at all in the genePred? */
    bool fHaveCds;

    /* were the start/end of CDS found in exons (start is genePred genomic
     * start) */
    bool fCdsStartInExon;
    bool fCdsEndInExon;

    /* status of start and end of CDS (strand) */
    Gene::CDSStatus fCdsStartStat;
    Gene::CDSStatus fCdsEndStat;

    /* per genePred exon frame info, either from genePred ext or computed */
    vector<int> fExonFrames;

    /* internal methods */
    const Coords& getChrom(const char *chrom);

    void createFeature(const Coords& chrom,
                       Gene* gene,
                       char strand,
                       int start,
                       int end,
                       Gene::Feature::Type type,
                       int frame = -1);
    void createExonWithCDS(const Coords& chrom,
                           Gene* gene,
                           int iExon);
    void createExon(const Coords& chrom,
                    Gene* gene,
                    int iExon);
    void createIntron(const Coords& chrom,
                      Gene* gene,
                      int iExon);
    void computeExonFrames();
    void setupExonFrames();
    void computeCdsStatus();
    void computeCDSInfo();
    Gene* toGene(const Coords& chrom);
    void warn(const string& msg);
    bool check(const Coords& chrom);

    public:
    /** Constructor */
    GenePredReading(const string& tabFile,
                   Genome* genome,
                   unsigned options);

    /* Destructor */
    ~GenePredReading();

    /* Set the intergenic size */
    void setIntergenic(unsigned size) {
        fIntergenic = size;
    }

    /** Read the next genePred, ignoring rows with errors.*/
    Gene* next();

    /** Get ptr to last genePred that was read */
    struct genePred* getGenePred() const {
        return fGenePred;
    }
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */

