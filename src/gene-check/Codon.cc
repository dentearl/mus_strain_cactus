#include "Codon.h"

// FIXME: amino being an enum make it not directly printable, etc.
// should be char constants.

/**
 * All-inserts init value.
 */
const string Codon::ALL_INSERTS("---");

/**
 * A table used to convert bases into amino acids the ordering of the bases is
 * given by the baseToIdx method.
 */
const Codon::Amino Codon::sBaseToAminoTable[4][4][4] = {
    {{PHE, PHE, LEU, LEU},
     {SER, SER, SER, SER},
     {TYR, TYR, STP, STP},
     {CYS, CYS, STP, TRP}},
    {{LEU, LEU, LEU, LEU},
     {PRO, PRO, PRO, PRO},
     {HIS, HIS, GLN, GLN},
     {ARG, ARG, ARG, ARG}},
    {{ILE, ILE, ILE, MET},
     {THR, THR, THR, THR},
     {ASN, ASN, LYS, LYS},
     {SER, SER, ARG, ARG}},
    {{VAL, VAL, VAL, VAL},
     {ALA, ALA, ALA, ALA},
     {ASP, ASP, GLU, GLU},
     {GLY, GLY, GLY, GLY}}
};


/**
 * Converts a base into a number that is then used to index into the base to
 * amino acid table.
 */
int Codon::baseToIdx(char b) const {
    switch (b) {
    case 'T':
    case 'U':
    case 't':
    case 'u':
	return 0;
    case 'C':
    case 'c':
	return 1;
    case 'A':
    case 'a':
	return 2;
    case 'G':
    case 'g':
	return 3;
    default:
	return -1;
    }
}

/**
 * Get the amino acid code for the codon.
 */
Codon::Amino Codon::getAmino() const {
    int aIdx = baseToIdx((*this)[0]);
    int bIdx = baseToIdx((*this)[1]);
    int cIdx = baseToIdx((*this)[2]);
    
    if((aIdx < 0) || (bIdx < 0) || (cIdx < 0)) {
	return XXX;
    } else {
        return sBaseToAminoTable[aIdx][bIdx][cIdx];
    }
}

/*
 * Is an amino acid four-fold degenerate
 */
bool Codon::isFourFoldDegenerate(Amino a) const {
    return (a == ALA) || (a == GLY) || (a == PRO) || (a == THR) || (a == VAL);
}

/*
 * Is this codon an amino acid that is a fourfold degenerate
 * of the fourfold case of a sixfold degenerate
 */
bool Codon::is4d() const {
    if (!isValid()) {
        return false;
    }

    Amino a = getAmino();
    if (isFourFoldDegenerate(a)) {
        return true;
    }

    // Check for sixfold degenerates
    char base0 = toupper((*this)[0]);
    return ((a == ARG) && (base0 == 'C'))
        || ((a == LEU) && (base0 == 'C'))
        || ((a == SER) && ((base0 == 'U' || base0 == 'T')));
}

