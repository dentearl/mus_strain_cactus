#ifndef CODON_H
#define CODON_H

#include <string>
#include "StringOps.h"

/**
 * String for storing a codon.
 * Most of the code here stolen from Krish.
 */
class Codon: public string {
  public:
    enum Amino {
        ALA  = 'A',  ARG  = 'R',  ASN  = 'N',  ASP  = 'D',  CYS  = 'C', 
        GLU  = 'E',  GLN  = 'Q',  GLY  = 'G',  HIS  = 'H',  ILE  = 'I', 
        LEU  = 'L',  LYS  = 'K',  MET  = 'M',  PHE  = 'F',  PRO  = 'P', 
        SER  = 'S',  THR  = 'T',  TRP  = 'W',  TYR  = 'Y',  VAL  = 'V', 
        STP  = '<',  // stop
        XXX  = 'X'   // unknown
    };

  private:
    /** All-inserts init value. */
    static const string ALL_INSERTS;
    /* amino acid mapping table */
    static const Amino sBaseToAminoTable[4][4][4];

    int baseToIdx(char b) const;
    bool isFourFoldDegenerate(Amino a) const;
  public:
    /** constructor */
    Codon():
        string(ALL_INSERTS) {
    }

    /** constructor from an offset in a string */
    Codon(const string& seq,
          int seqIdx):
        string(ALL_INSERTS) {
        int left =  seq.size() - seqIdx;
        if (left > 3) {
            left = 3;
        }
        for (int i = 0; i < left; i++) {
            (*this)[i] = seq[seqIdx+1];
        }
    }

    /** Get the amino acid code for the codon. */
    Amino getAmino() const;

    /** Is this a valid codon (doesn't contain deletions) */
    bool isValid() const {
        return !(((*this)[0] == '-') || ((*this)[1] == '-')
                 || ((*this)[2] == '-'));
    }

    /** Is this a start codon */
    bool isStart() const {
        return isValid() && StringOps::equalIgnoreCase(*this, "ATG");
    }

    /** Is this a stop codon. */
    bool isStop() const {
        return isValid()
            && (StringOps::equalIgnoreCase(*this, "TAA")
                || StringOps::equalIgnoreCase(*this, "TAG")
                || StringOps::equalIgnoreCase(*this, "TGA"));
    }

    /*
     * Is this codon an amino acid that is a fourfold degenerate
     * of the fourfold case of a sixfold degenerate
     */
    bool is4d() const;

};
#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */
