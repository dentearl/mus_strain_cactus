#ifndef GENE_TABLE_H
#define GENE_TABLE_H

using namespace std;
#include <typeinfo>
#include <vector>
#include "Gene.h"
#include "CoordRangeMap.h"
#include "StringMap.h"

/**
 * Vector of genes, sorted into assending order, with has of gene names.
 */
class GeneTable {
    public:
    static const unsigned RANGE_INDEXED = 0x01;
    static const unsigned OWNS_OBJECTS =   0x02;

    private:
    unsigned fOptions;

    /* Vector of genes */
    vector<Gene*> fGenes;

    /* map of genes by name */
    StringMap<Gene*> fGeneMap;

    /* map of gene ranges */
    CoordRangeMap<Gene*>* fRangeMap;

    /* free function for genes */
    static void freeGene(Gene** genePtr) {
        if (*genePtr != NULL) {
            delete *genePtr;
            *genePtr = NULL;
        }
    }

    public:
    /* constructor */
    GeneTable(unsigned options);

    /* destructor */
    ~GeneTable();

    /* Get a gene by name, or NULL if not found */
    Gene* find(const string& name) {
        return fGeneMap.get(name);
    }
    
    /* add a gene */
    void add(Gene* gene);

    /** accessors */
    unsigned size() const {
        return fGenes.size();
    }
    Gene* operator[](unsigned idx) {
        assert(idx < fGenes.size());
        return fGenes[idx];
    }
    
    /** sort the table into assending order */
    void sort();

    /** Get list of genes overlaping a range. */
    void getOverlaping(const Coords& range,
                       vector<Gene*>& genes);

    /** Get list of genes completely contained in range. */
    void getContained(const Coords& range,
                      vector<Gene*>& genes);
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
