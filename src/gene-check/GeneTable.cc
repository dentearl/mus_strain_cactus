#include "GeneTable.h"
#include "FatalError.h"

/* constructor */
GeneTable::GeneTable(unsigned options):
    fOptions(options),
    fRangeMap(NULL) {
    if (fOptions & RANGE_INDEXED) {
        if (fOptions & OWNS_OBJECTS) {
            fRangeMap = new CoordRangeMap<Gene*>(Coords::STRAND, freeGene);
        } else {
            fRangeMap = new CoordRangeMap<Gene*>(Coords::STRAND);
        }
    }
}

/* destructor */
GeneTable::~GeneTable() {
    delete fRangeMap;
}

/* add a gene */
void GeneTable::add(Gene* gene) {
    fGenes.push_back(gene);
    fGeneMap.insert(gene->getName(), gene);
    if (fRangeMap != NULL) {
        fRangeMap->add(gene->getCoords(), gene);
    }
}

static int sortCmp(const void* cell1,
                   const void* cell2) {
    return (*(Gene**)cell1)->getCoords().compare((*(Gene**)cell2)->getCoords());
}

/** 
 * sort the table into assending order
 */
void GeneTable::sort() {
    qsort(&(fGenes)[0], fGenes.size(), sizeof(Gene*), sortCmp);
}

/**
 * Get list of genes overlaping a range.
 */
void GeneTable::getOverlaping(const Coords& range,
                              vector<Gene*>& genes) {
    if (fRangeMap != NULL) {
        new FatalError("GeneTable doesn't have range index");
    }
    fRangeMap->getOverlaping(range, genes);
}

/**
 * Get list of genes completely contained in range.
 */
void GeneTable::getContained(const Coords& range,
                             vector<Gene*>& genes) {

    vector<Gene*> overlaping;
    getOverlaping(range, overlaping);
    for (int i = 0; i < overlaping.size(); i++) {
        Gene* gene = overlaping[i];
        if (range.contains(gene->getCoords())) {
            genes.push_back(gene);
        }
    }
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */
