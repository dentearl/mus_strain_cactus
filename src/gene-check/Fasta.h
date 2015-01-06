#ifndef FASTA_FASTA_H
#define FASTA_FASTA_H

#include <typeinfo>
#include <string>
#include <vector>
#include "FIOStream.h"

/**
 * Fasta file reader and writer.
 *
 * <P> A index file may optionally be supplied to allow random access to
 * files.  The index is created by using the gnu grep `-b' option to grep for
 *  '>':
 * <PRE>
 *    grep -b '>' seqs.fasta >seqs.faindex
 * </PRE>
 * resulting in a file in the format:
 * <PRE>
 * 0:>gnl|ti|16064096 jkp68h09.g1
 * 721:>gnl|ti|16064102 jkq29c04.b1
 * 1421:>gnl|ti|16064124 jkq08a12.g1
 * </PRE>
 * Blank lines and lines with the first non-white-space character being
 * `#' are skipped.  Comment following id is optional.
 */
class Fasta {
  public:
    /** Read access */
    static const int READ = 1;

    /** Write access */
    static const int WRITE = 2;

    /** Max length of a line of the sequence to write */
    static const int MAX_LINE_LENGTH = 72;

    /**
     * Interface for filter objects.  A filter can accept or reject
     * a record or can edit the data in a record before its returned.
     * This can be used to normalize sequence ids, add information to
     * the comment, etc.
     */
    class Filter {
      public:
        /**
         * Process a record.  Any argument can be edited.
         * @param seqId The sequence id
         * @param comment The sequence comment
         * @param sequence The sequence data.
         * @return true if the sequence should be accept, false if it
         *  should be rejected.
         */
        virtual bool filter(string seqId,
                            string comment,
                            string data) = 0;
        /** destructor */
        virtual ~Filter() {
        }
    };

  private:
    class IndexEntry;
    class RecIndex;

    // Characters used to separate id from comment
    static const string ID_TERMINATORS;

    // Filter list.
    vector<Filter*> fFilters;

    // File being read.
    FIOStream* fStream;
    bool fOwnStream;    // did we open it?
    int fLineNum;

    // Line buffer.  If not empty, this line contains the next line
    // to use.  Needed due to not knowing the end of a record until
    // the next record is read.
    string fLineBuf;

    // Current record.
    string fSeqId;
    string fComment;
    string fData;

    // Does the object currently have a sequence.
    bool fHaveSequence;

    // If not null, then the index for random read access.
    RecIndex* fRecIndex;
    
    /*
     * Read a line into the buffer.
     */
    bool readLine();

    /*
     * Parse the id/comment line.
     */
    void parseId();

    /**
     * Read the next record into the object.
     * @return true if read, false if EOF.
     */
    bool readNextRec();

    /** Run filters on current sequence. */
    bool runFilters();

public:
    /**
     * Constructor.
     *
     * @param fileName Fasta file name.  The file maybe compressed.
     * @param mode READ or WRITE
     * @param indexFileName If not the empty string, this is the name of
     *  the index file.
     */
    Fasta(const string& fileName,
          int mode = READ,
          const string& indexFileName = "");
    
    /**
     * Constructor from open file.
     *
     * @param fastStream open file stream
     * @param mode READ or WRITE
     * @param lineNum current line number, or 0 if not known
     */
    Fasta(FIOStream* fastaStream,
          int mode = READ,
          int lineNum = 0);
    
    /** Destructor. */
    ~Fasta();

    /** Add a filter.  Ownership of the object will be transfered. */
    void addFilter(Filter* filter) {
        fFilters.push_back(filter);
    }

    /** Get the file name */
    const string& getFileName() const {
        return fStream->getFileName();
    }

    /**
     * Read the next available record into the object.
     * @return true if read, false if EOF.
     */
    bool readRec();

    /**
     * Read a record randomly given it's ID.  An index file must have been
     * specified at open.  The next call to readRec will return the
     * sequence.
     *
     * @param seqId Id of the seq to find.
     * @param noError If true, return status instead of generating an
     *  error if the sequence is not found.  Optional, default is false.
     * @return true if the sequence was found, false if the sequence
     *  was not found and <code>noError</code> was <code>true</code>.
     */
    bool readRec(const string& seqId,
                 bool noError = false);

    /** Test if there is currently a sequence in the object. */
    bool haveSequence() {
        return fHaveSequence;;
    }

    /** Get the seqId of the current record. */
    const string& getSeqId() const {
        return fSeqId;
    }

    /** Get the comment of the current record. */
    const string& getComment() const {
        return fComment;
    }

    /** Get the length of the current sequence. */
    int getLength() const {
        return (int)fData.size();
    }

    /** Get the sequence data the current record. */
    const string& getData() const {
        return fData;
    }

    /** Write a sequence. */
    void writeRec(const string& id,
                  const string& comment,
                  const string& seqString);

};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */

