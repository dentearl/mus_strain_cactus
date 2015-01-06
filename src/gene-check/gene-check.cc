/*
 * gene-check - gene validation program for building training sets.
 */
#include "CmdOptions.h"
#include "GenePredReading.h"
#include "GeneChecker.h"
#include "Genome.h"
#include "FatalError.h"
#include "StringVector.h"
#include "StringStringMap.h"
#include "FileOps.h"
#include "FIOStream.h"
#include "Fasta.h"


static string usageMsg(
    "[options] inFile outTsv\n"
    "Arguments:\n"
    "  o inFile - Eihter a genePred format tab file, which  must be grouped\n"
    "    by chromosome or a fasta file with the mRNA sequences.\n");

extern "C" {
#include "common.h"
#include "genbank.h"
#include "genePred.h"
}

static const StringCmdOptionDef OPT_GENOME_SEQS(
    "--genome-seqs", "path - Directory containing NIB files or path to two-bit file for geomoe sequences (required).");

static const BoolCmdOptionDef OPT_NO_SPLICE_CHECK(
    "--no-splice-check", "- don't check intron splice sites");

static const BoolCmdOptionDef OPT_NO_CDS_CHECK(
    "--no-cds-check", "- don't check CDS attributes");

static const BoolCmdOptionDef OPT_CANONICAL_SPLICE(
    "--canonical-splice", "- only allow canonical splice sites");

static const BoolCmdOptionDef OPT_MRNA_FASTA_CDS(
    "--mrna-fasta-cds", "- input is a fasta file of mRNA with CDS in upper case");

static const BoolCmdOptionDef OPT_MRNA_FASTA(
    "--mrna-fasta", "- input is an fasta file of mRNA, must specify CDS file");

static const StringCmdOptionDef OPT_CDS_FILE(
    "--cds-file", "cds -  File is tab-seperate file with two columns, the first"
    " being the sequence id, the second being the genbank CDS specification.");

static const BoolCmdOptionDef OPT_PROB_ONLY(
    "--prob-only", "- only output genes with problems");

static const BoolCmdOptionDef OPT_CDS_ONLY(
    "--cds-only", "- only validate within CDS.  UTR splice problems are still counted, however "
    "they don't result in errors.");

static const StringCmdOptionDef OPT_OK_GENEPRED_OUT(
    "--ok-genepred-out", "gpfile -  output genePred rows that pass tests to this file.");

static const StringCmdOptionDef OPT_DETAILS_OUT(
    "--details-out", "file -  output file with details of each error detected.");

static const BoolCmdOptionDef OPT_NMD(
    "--nmd", " - NMD candidates are flagged as errors");

static const CmdOptionDef* optionsDef[] = {
    &OPT_GENOME_SEQS,
    &OPT_NO_SPLICE_CHECK,
    &OPT_NO_CDS_CHECK,
    &OPT_CANONICAL_SPLICE,
    &OPT_MRNA_FASTA_CDS,
    &OPT_MRNA_FASTA,
    &OPT_CDS_FILE,
    &OPT_PROB_ONLY,
    &OPT_CDS_ONLY,
    &OPT_OK_GENEPRED_OUT,
    &OPT_DETAILS_OUT,
    &OPT_NMD,
    NULL
};

/* Parse from command line */
static unsigned gCheckOpts = 0;
static bool gProbOnly = FALSE;

// new headers
static const string hdr1 = "acc\t"
    "chr\t"               "chrStart\t"            "chrEnd\t"
    "strand\t"            "stat\t"                "frame\t"
    "start\t"             "stop\t"                "orfStop\t"
    "cdsGap\t"            "cdsMult3Gap\t"         "utrGap\t"
    "cdsUnknownSplice\t"  "utrUnknownSplice\t"    "cdsNonCanonSplice\t"
    "utrNonCanonSplice\t"
    "numExons\t"          "numCds\t"              "numUtr5\t"
    "numUtr3\t"           "numCdsIntrons\t"       "numUtrIntrons\t"
    "nmd\t"               "causes";

/* object to count number of various features */
// FIXME: make functions on gene
class FeatureCnts {
    public:
    unsigned numExons;
    unsigned numCds;
    unsigned numUtr5;
    unsigned numUtr3;
    unsigned numCdsIntrons;
    unsigned numUtrIntrons;
    unsigned numCdsGaps;
    unsigned numUtrGaps;
    unsigned numCdsMult3Gaps;

    private:

    /* check if a feature is a gap */
    bool isGap(const Gene::Feature* feat) {
        // FIXME: make minIntron an agument
        return (feat->getType() & Gene::Feature::INTRON)
            && (feat->getLength() < GeneChecker::DEFAULT_MIN_INTRON);
    }


    /* determine if an intron is a CDS intron, which CDS on both sides, also
     * determined if a gap is in CDS */
    bool isCdsIntron(const Gene::Feature* feat) {
        const Gene::Feature* prev = feat->getPrev();
        const Gene::Feature* next = feat->getNext();
        return (feat->getType() & Gene::Feature::INTRON)
            && (prev != NULL) && (next != NULL)
            && (prev->getType() & Gene::Feature::CDS)
            && (next->getType() & Gene::Feature::CDS);
    }

    /* get pointer to previous non-gap feature */
    const Gene::Feature* getPrevNonGapFeat(const Gene::Feature* feat) {
        const Gene::Feature* prev = feat->getPrev();
        while ((prev != NULL) && isGap(prev)) {
            prev = prev->getPrev();
        }
        return prev;
    }

    /* count an exon feature */
    void countExon(const Gene::Feature* feat) {
        const Gene::Feature* prevNonGap = getPrevNonGapFeat(feat);
        // for biological exons, count only first exon feature
        if ((prevNonGap == NULL)
            || ((prevNonGap->getType() & Gene::Feature::EXON_MASK) == 0)) {
            numExons++;
        }
        if ((prevNonGap == NULL)
            || ((prevNonGap->getType() & Gene::Feature::EXON_MASK)
                != (feat->getType() & Gene::Feature::EXON_MASK))) {
            /* not a continuation after a gap, count by feature type */
            if (feat->getType() & Gene::Feature::CDS) {
                numCds++;
            } else if (feat->getType() & Gene::Feature::UTR5) {
                numUtr5++;
            } else if (feat->getType() & Gene::Feature::UTR3) {
                numUtr3++;
            }
        }
    }

    /* count an intron feature */
    void countIntron(const Gene::Feature* feat) {
        // CDS intron has CDS on both sides, otherwise it's a UTR intron 
        if (isCdsIntron(feat)) {
            numCdsIntrons++;
        } else {
            numUtrIntrons++;
        }
    }

    /* count an gap feature (which is currently stored as an intron) */
    void countGap(const Gene::Feature* feat) {
        if (isCdsIntron(feat)) {
            numCdsGaps++;
            if ((feat->getLength() % 3) == 0) {
                numCdsMult3Gaps++;
            }
        } else {
            numUtrGaps++;
        }
    }

    public:
    /* constructor */
    FeatureCnts(Gene * gene) :
        numExons(0),
        numCds(0),
        numUtr5(0),
        numUtr3(0),
        numCdsIntrons(0),
        numUtrIntrons(0),
        numCdsGaps(0),
        numUtrGaps(0),
        numCdsMult3Gaps(0) {

        int nFeats = gene->getNumRealFeatures();
        for (int iFeat = 0; iFeat < nFeats; iFeat++) {
            const Gene::Feature* feat = gene->getRealFeature(iFeat);
            if (feat->getType() & Gene::Feature::EXON_MASK) {
                countExon(feat);
            } else if (isGap(feat)) {
                countGap(feat);
            } else {
                countIntron(feat);
            }
        }
    }
};

/*
 * Output check results
 */
static void outputResults(Gene* gene,
                          GeneChecker& checker,
                          ostream& out) {
    StringVector causes; 
    Coords coords(gene->getCoords(), Coords::GENOMIC);
    unsigned probs = checker.getProblems();
    unsigned errs = checker.getErrors();
    FeatureCnts featCnts(gene);

    assert(featCnts.numCdsGaps == checker.getNumCdsGaps());
    assert(featCnts.numUtrGaps == checker.getNumUtrGaps());
    assert(featCnts.numCdsMult3Gaps <= featCnts.numCdsGaps);

    out << gene->getName() << '\t'
        << coords.getName()  << '\t'
        << coords.getStart()  << '\t'
        << coords.getEnd()  << '\t'
        << ((coords.getStrand() == Coords::NO_STRAND) ? '.' : coords.getStrand());
    if (errs == 0) {
        out << "\tok";
    } else {
        out << "\terr";
    }
    if (probs & GeneChecker::NO_CDS) {
        out << "\tnoCDS";
        if (errs & GeneChecker::NO_CDS) {
            causes.add(checker.getProblemSym(GeneChecker::NO_CDS));
        }
    } else if (probs & GeneChecker::BAD_FRAME) {
        out << "\tbad";
        if (errs & GeneChecker::BAD_FRAME) {
            causes.add(checker.getProblemSym(GeneChecker::BAD_FRAME));
        }
    } else if (probs & GeneChecker::FRAME_MISMATCH) {
        out << "\tmismatch";
        if (errs & GeneChecker::FRAME_MISMATCH) {
            causes.add(checker.getProblemSym(GeneChecker::FRAME_MISMATCH));
        }
    } else if (probs & GeneChecker::FRAME_DISCONTIG) {
        out << "\tdiscontig";
        if (errs & GeneChecker::FRAME_DISCONTIG) {
            causes.add(checker.getProblemSym(GeneChecker::FRAME_DISCONTIG));
        }
    } else {
        out << "\tok";
    }
    if (probs & GeneChecker::NO_START_CODON) {
        out << "\tno";
        if (errs & GeneChecker::NO_START_CODON) {
            causes.add(checker.getProblemSym(GeneChecker::NO_START_CODON));
        }
    } else {
        out << "\tok";
    }
    if (probs & GeneChecker::NO_STOP_CODON) {
        out << "\tno";
        if (errs & GeneChecker::NO_STOP_CODON) {
            causes.add(checker.getProblemSym(GeneChecker::NO_STOP_CODON));
        }
    } else {
        out << "\tok";
    }
    out << "\t" << checker.getNumInFrameStop();
    if (errs & GeneChecker::IN_FRAME_STOP_CODON) {
        causes.add(checker.getProblemSym(GeneChecker::IN_FRAME_STOP_CODON));
    }
    out << "\t" << checker.getNumCdsGaps();
    if (errs & GeneChecker::CDS_GAP) {
        causes.add(checker.getProblemSym(GeneChecker::CDS_GAP));
    }
    out << "\t" << featCnts.numCdsMult3Gaps;
    out << "\t" << checker.getNumUtrGaps();
    if (errs & GeneChecker::UTR_GAP) {
        causes.add(checker.getProblemSym(GeneChecker::UTR_GAP));
    }
    out << "\t" << checker.getNumUnknownCdsIntrons();
    if (errs & GeneChecker::CDS_UNKNOWN_SPLICE) {
        causes.add(checker.getProblemSym(GeneChecker::CDS_UNKNOWN_SPLICE));
    }
    out << "\t" << checker.getNumUnknownUtrIntrons();
    if (errs & GeneChecker::UTR_UNKNOWN_SPLICE) {
        causes.add(checker.getProblemSym(GeneChecker::UTR_UNKNOWN_SPLICE));
    }
    out << "\t" << checker.getNumNonCanonicalCdsIntrons();
    if (errs & GeneChecker::CDS_NONCANON_SPLICE) {
        causes.add(checker.getProblemSym(GeneChecker::CDS_NONCANON_SPLICE));
    }
    out << "\t" << checker.getNumNonCanonicalUtrIntrons();
    if (errs & GeneChecker::UTR_NONCANON_SPLICE) {
        causes.add(checker.getProblemSym(GeneChecker::UTR_NONCANON_SPLICE));
    }
    out << "\t" << featCnts.numExons
        << "\t" << featCnts.numCds
        << "\t" << featCnts.numUtr5
        << "\t" << featCnts.numUtr3
        << "\t" << featCnts.numCdsIntrons
        << "\t" << featCnts.numUtrIntrons;
    if (probs & GeneChecker::NMD) {
        out << "\tnmd";
        if (errs & GeneChecker::NMD) {
            causes.add(checker.getProblemSym(GeneChecker::NMD));
        }
    } else {
        out << "\tok";
    }
    out << "\t" << causes.join(',');
    out << endl;
}

/*
 * Check a gene, output the results.
 */
static bool checkGene(Gene* gene,
                      GeneChecker& checker,
                      ostream& out) {
    bool isOk = checker.fullCheck(gene);
    if ((!isOk) || (!gProbOnly)) {
        outputResults(gene, checker, out);
    }
    return isOk;
}

/*
 * Check gene data from gene a genePred file
 */
static void genePredCheck(const string& genePredTab,
                          const string& genomeSeqs,
                          ostream& out,
                          FILE* okGenePredOut,
                          ostream* detailsOut) {
    Genome *genome = Genome::loadFromGenome(genomeSeqs);
    GenePredReading geneReader(genePredTab, genome,
                              GenePredReading::VERBOSE_ERRORS|GenePredReading::READ_SEQS);
    GeneChecker checker(gCheckOpts, detailsOut);
    Gene* gene;
    while ((gene = geneReader.next()) != NULL) {
        if (checkGene(gene, checker, out)) {
            if (okGenePredOut != NULL) {
                genePredTabOut(geneReader.getGenePred(), okGenePredOut);
            }
        }
        delete gene;
    }
    delete genome;
}

/*
 * Find the next character in the given range, return string size of not
 * found.
 */
unsigned nextCharInRange(const string& str,
                         unsigned nextIdx,
                         char firstChar,
                         char lastChar) {
    unsigned sz = str.size();
    while ((nextIdx < sz)
           && !((firstChar <= str[nextIdx]) && (str[nextIdx] <= lastChar))) {
        nextIdx++;
    }
    return nextIdx;
}

/*
 * Create a gene from an mRNA sequence with CDS as upper case.
 */
static Gene* geneFromMRnaSeq(const string& acc,
                             const string& mrnaSeq) {
    unsigned len = mrnaSeq.size();
    unsigned cdsStart = nextCharInRange(mrnaSeq, 0, 'A', 'Z');
    if (cdsStart == len) {
        throw FatalError("no CDS for " + acc);
    }
    unsigned cdsAfter = nextCharInRange(mrnaSeq, cdsStart, 'a', 'z');
    if (nextCharInRange(mrnaSeq, cdsAfter, 'A', 'Z') != len) {
        throw FatalError("multiple upper case CDS annotations for " + acc);
    }
    Coords mrnaCoords(acc, Coords::STRAND, Coords::NO_STRAND, 0, len, len);
    Gene *gene = new Gene(acc);
    if (cdsStart > 0) {
        gene->addFeature(Gene::Feature::UTR5,
                         Coords(mrnaCoords, 0, cdsStart));
    }
    gene->addFeature(Gene::Feature::CDS,
                     Coords(mrnaCoords, cdsStart, cdsAfter), 0);
    if (cdsAfter < len) {
        gene->addFeature(Gene::Feature::UTR3,
                         Coords(mrnaCoords, cdsAfter, len));
    }
    gene->completeFeatures();
    gene->setSeq(mrnaSeq);
    return gene;
}

/*
 * Check gene data in mRNA sequences with the CDS in upper case
 */
static void mrnaFastaCdsCheck(const string& mrnaFasta,
                              ostream& out,
                              ostream* detailsOut) {
    GeneChecker checker(gCheckOpts, detailsOut);
    Fasta fa(mrnaFasta, Fasta::READ);
    while (fa.readRec()) {
        Gene* gene = geneFromMRnaSeq(fa.getSeqId(), fa.getData());
        if (gene != NULL) {
            checkGene(gene, checker, out);
            delete gene;
        }
    }
}

/*
 * Load CDS from a tab-seperated file
 */
static void loadCdsFile(const string& file,
                        StringStringMap& cdsTable) {
    FIOStream in(file);
    string line;
    while (getline(in, line).good()) {
        size_t tabIdx = line.find('\t');
        if (tabIdx == string::npos) {
            throw FatalError("line does not contain tab in " + in.getFileName());
        }
        string acc(line.substr(0, tabIdx));
        string cdsStr(line.substr(tabIdx+1));
        cdsTable.insert(acc, cdsStr);
    }
}

/* lookup and parse cds for a gene */
static bool parseCds(const string& acc,
                     const StringStringMap& cdsTable,
                     unsigned *cdsStart,
                     unsigned *cdsEnd) {
    const string* cdsStr = cdsTable.get(acc);
    if (cdsStr == NULL) {
        cerr << "Warning: no CDS found for " << acc << endl;
        return false;
    }
    if (!genbankParseCds((char*)cdsStr->c_str(), cdsStart, cdsEnd)) {
        cerr << "Warning: invalid CDS string for " << acc << ": " 
             << cdsStr << endl;
        return false;
    }
    return true;
}

/*
 * Create a gene from an mRNA sequence with CDS from a table
 */
static Gene* geneFromMRna(const string& acc,
                          const string& mrnaSeq,
                          const StringStringMap& cdsTable) {
    unsigned cdsStart, cdsEnd;
    if (!parseCds(acc, cdsTable, &cdsStart, &cdsEnd)) {
        return NULL;
    }
    unsigned len = mrnaSeq.size();
    Coords mrnaCoords(acc, Coords::STRAND, Coords::NO_STRAND, 0, len, len);
    Gene *gene = new Gene(acc);
    if (cdsStart > 0) {
        gene->addFeature(Gene::Feature::UTR5,
                         Coords(mrnaCoords, 0, cdsStart));
    }
    gene->addFeature(Gene::Feature::CDS,
                     Coords(mrnaCoords, cdsStart, cdsEnd), 0);
    if (cdsEnd < len) {
        gene->addFeature(Gene::Feature::UTR3,
                         Coords(mrnaCoords, cdsEnd, len));
    }
    gene->completeFeatures();
    gene->setSeq(mrnaSeq);
    return gene;
}

/*
 * Check gene data in mRNA sequences with in an external file.
 */
static void mrnaFastaCheck(const string& mrnaFasta,
                           const string& cdsFile,
                           ostream& out,
                           ostream* detailsOut) {
    StringStringMap cdsTable;
    loadCdsFile(cdsFile, cdsTable);
    GeneChecker checker(gCheckOpts, detailsOut);
    Fasta fa(mrnaFasta, Fasta::READ);
    while (fa.readRec()) {
        Gene* gene = geneFromMRna(fa.getSeqId(), fa.getData(), cdsTable);
        if (gene != NULL) {
            checkGene(gene, checker, out);
            delete gene;
        }
    }
}

/**
 * Main
 */
int main(int argc,
         const char* const* argv) {
    // Parse options.
    CmdOptions opts(2, 2, usageMsg, optionsDef);
    opts.parse(argc, argv);

    string inFile = opts.getPositionalArg(0);
    string outTsv = opts.getPositionalArg(1);

    // start with defaults so we automatically get new options
    gCheckOpts = GeneChecker::ALL_OPTIONS;
    if (!opts.specified(&OPT_CANONICAL_SPLICE)) {
        gCheckOpts &= ~(GeneChecker::CDS_NONCANON_SPLICE|GeneChecker::UTR_NONCANON_SPLICE);
    }
    if (opts.specified(&OPT_NO_SPLICE_CHECK)) {
        gCheckOpts &= ~(GeneChecker::CDS_NONCANON_SPLICE|GeneChecker::UTR_NONCANON_SPLICE|GeneChecker::CDS_UNKNOWN_SPLICE|GeneChecker::UTR_UNKNOWN_SPLICE);
    }
    if (opts.specified(&OPT_NO_CDS_CHECK)) {
        gCheckOpts &= ~(GeneChecker::BAD_FRAME|GeneChecker::NO_START_CODON|GeneChecker::NO_STOP_CODON|GeneChecker::IN_FRAME_STOP_CODON|GeneChecker::NO_CDS|GeneChecker::FRAME_MISMATCH|GeneChecker::FRAME_DISCONTIG|GeneChecker::NMD);
    }
    if (opts.specified(&OPT_CDS_ONLY)) {
        gCheckOpts &= ~(GeneChecker::UTR_NONCANON_SPLICE|GeneChecker::UTR_UNKNOWN_SPLICE|GeneChecker::UTR_GAP);
    }
    if (!opts.specified(&OPT_NMD)) {
        gCheckOpts &= ~GeneChecker::NMD;
    }
    gProbOnly = opts.specified(&OPT_PROB_ONLY);
    if (opts.specified(&OPT_MRNA_FASTA) && !opts.specified(&OPT_CDS_FILE)) {
        throw FatalError("must specify " + OPT_CDS_FILE.getName() + " with " + OPT_MRNA_FASTA.getName());
    }

    FIOStream out(outTsv, ios::out);
    out << hdr1 << endl;

    FIOStream* detailsOut = NULL;
    if (opts.specified(&OPT_DETAILS_OUT)) {
        detailsOut = new FIOStream(opts.getStringValue(&OPT_DETAILS_OUT), ios::out);
        *detailsOut << GeneChecker::DETAILS_HDR1 << endl;
    }

    if (opts.specified(&OPT_MRNA_FASTA_CDS)) {
        mrnaFastaCdsCheck(inFile, out, detailsOut);
    } else if (opts.specified(&OPT_MRNA_FASTA)) {
        mrnaFastaCheck(inFile, opts.getStringValue(&OPT_CDS_FILE), out, detailsOut);
    } else if (opts.specified(&OPT_GENOME_SEQS)) {
        FILE* okGenePredOut = NULL;
        if (opts.specified(&OPT_OK_GENEPRED_OUT)) {
            okGenePredOut = mustOpen((char*)opts.getStringValue(&OPT_OK_GENEPRED_OUT).c_str(), (char*)"w");
        }
        genePredCheck(inFile, opts.getStringValue(&OPT_GENOME_SEQS), out, okGenePredOut, detailsOut);
        carefulClose(&okGenePredOut);
    } else {
        throw FatalError("must specify one of -" + OPT_MRNA_FASTA_CDS.getName()
                         + ", -" + OPT_MRNA_FASTA.getName() + " or -" + OPT_GENOME_SEQS.getName());
    }
    delete detailsOut;
    return 0;
}
