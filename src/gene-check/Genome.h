#ifndef GENOME_H
#define GENOME_H
using namespace std;
#include "Coords.h"
#include "StringMap.h"
struct fileInfo;
struct nibTwoCache;

/**
 * Access to genome chromsome information and sequences.
 */
class Genome {
    public:
    /** information about one chromsome */
    class Chrom: public Coords {
        friend class Genome;
        private:
        Genome *fGenome;

        /* constructor */
        Chrom(Genome *genome,
              const string& chrom,
              unsigned length) :
            Coords(chrom, Coords::GENOMIC, Coords::NO_STRAND,
                   0, length, length),
            fGenome(genome) {
        }

        public:
        /* get associated genome object */
        Genome* getGenome() {
            return fGenome;
        }
    };

    private:
    /* tables of Chrom objects */ 
    StringMap<Chrom*> fChromMap;
    vector<Chrom*> fChroms;

    /* object used to access twoBit of nib files; keeps twoBit open */
    struct nibTwoCache *fCache;

    void add(const string& name, 
             unsigned length);
    void addFromNib(struct fileInfo* nib);
    void loadFromNibDir(const string& genomeSpec);
    void loadFromTwoBit(const string& genomeSpec);

    /* constructor */
    Genome():
        fCache(NULL) {
    }

    public:

    /* get number of Chroms */
    unsigned size() const {
        return fChroms.size();
    }

    /* Get a Chrom object by name */
    Chrom* getChrom(const string& name) const;

    /* Get a Chrom object by index */
    Chrom* getChrom(unsigned idx) const {
        return fChroms[idx];
    }

    /**
     * Read a sequence.  Reverse complementing if strand coordinates and
     * the negative strand.
     */
    string read(const Coords& coords);

    /* Factory to build from twoBit file or nib directory */
    static Genome* loadFromGenome(const string& genomeSpec);
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */

