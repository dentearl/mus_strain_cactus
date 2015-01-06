#include "Genome.h"
#include "StringOps.h"
#include "FatalError.h"
#include "FileOps.h"

extern "C" {
#include "common.h"
#include "portable.h"
#include "linefile.h"
#include "chromInfo.h"
#include "nib.h"
#include "twoBit.h"
#include "nibTwo.h"
#include "dnautil.h"
#include "dnaseq.h"
}

// FIXME: leaks memory, needs destructor

/* add a Chrom object */
void Genome::add(const string& name, 
                 unsigned length) {
    Chrom *chrom = new Chrom(this, name, length);
    fChromMap.insert(chrom->getName(), chrom);
    fChroms.push_back(chrom);
}

/* add a chrom given a nib file */
void Genome::addFromNib(struct fileInfo* nib) {
    /* parse chrom name from file */
    string name = FileOps::root(FileOps::tail(nib->name));
    FILE* fh;
    int length;
    nibOpenVerify(nib->name, &fh, &length);
    fclose(fh);
    add(name, length);
}

/** load from nib directory */
void Genome::loadFromNibDir(const string& genomeSpec) {
    struct fileInfo *nibFiles = listDirX((char*)genomeSpec.c_str(), (char*)"*.nib", true);
    if (nibFiles == NULL) {
        throw FatalError("no nibs found in " + genomeSpec);
    }
    for (struct fileInfo *nib = nibFiles; nib != NULL; nib = nib->next) {
        addFromNib(nib);
    }
    slFreeList(&nibFiles);
}

/** load from twoBit file */
void Genome::loadFromTwoBit(const string& genomeSpec) {
    for (struct twoBitIndex *tbi = fCache->tbf->indexList; tbi != NULL; tbi = tbi->next) {
        add(tbi->name, twoBitSeqSize(fCache->tbf, tbi->name));
    }
}

/** Factory to build from twoBit file or nib directory */
Genome* Genome::loadFromGenome(const string& genomeSpec) {
    dnaUtilOpen(); // must call at least once
    Genome* genome = new Genome();
    genome->fCache = nibTwoCacheNew(const_cast<char*>(genomeSpec.c_str()));
    if (genome->fCache->isTwoBit) {
        genome->loadFromTwoBit(genomeSpec);
    } else {
        genome->loadFromNibDir(genomeSpec);
    }
    return genome;
}

/* Get a Chrom object by name */
Genome::Chrom* Genome::getChrom(const string& name) const {
    Chrom* chrom = fChromMap.get(name);
    if (chrom == NULL) {
        throw FatalError("chromosome not found in genome table: " + name);
    }
    return chrom;
}

/**
 * Read a sequence.  Reverse complementing if strand coordinates and
 * the negative strand.
 */
string Genome::read(const Coords& coords) {
    Coords chromCoords(coords, Coords::GENOMIC);  /* postive strand */
    struct dnaSeq *dna = nibTwoCacheSeqPart(fCache,
                                            const_cast<char*>(chromCoords.getName().c_str()),
                                            chromCoords.getStart(), chromCoords.getLength(), NULL);
    if ((coords.getSystem() == Coords::STRAND)
        && (coords.getStrand() == Coords::NEG_STRAND)) {
        reverseComplement(dna->dna, dna->size);
    }
    // FIXME: is there a way to do this without copy, maybe with allocator?
    string seq(dna->dna);
    dnaSeqFree(&dna);
    return seq;
}

