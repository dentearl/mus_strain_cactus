#include "Fasta.h"
#include "StringOps.h"
#include "StringMap.h"
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "IOException.h"


// FIXME: grep -bn would include line number, this currently is lost when
// seeking.

// FIXME: is filter stuff useful or used?

/*
 * Characters used to separate id from comment.  Include all whitespace.
 */
const string Fasta::ID_TERMINATORS = "\011\012\013\014\015\040,;";

/** 
 * Entry in the index hash table
 */
class Fasta::IndexEntry {
    public:
    const string fSeqId;
    const int fByteOffset;

    /** constructor */
    IndexEntry(const string& seqId,
               int byteOffset):
        fSeqId(seqId),
        fByteOffset(byteOffset) {
    }
};

/**
 * Hash tabled used to manage an index of sequence id to record byte
 * offset.
 */
class Fasta::RecIndex: private StringMap<IndexEntry*> {
  private:
    /** Generate an error for an invalid line */
    void invalidEntry(const string& line,
                      const string& indexFileName,
                      int lineNum) {
        throw IOException("invalid fasta index file entry: \""
                          + line + "\"", indexFileName, lineNum);
    }

    /** Parse a line in the index file */
    void parseEntry(const string& line,
                    const string& indexFileName,
                    int lineNum) {
        // check if blank or comment
        size_t iLine = line.find_first_not_of(StringOps::WHITE_SPACE);
        if ((iLine == string::npos) || (line[iLine] == '#')) {
            return; // skip
        }

        /*
         * parse: 1421:>gnl|ti|16064124 jkq08a12.g1
         */

        // Offset
        size_t iNext = line.find_first_of(':', iLine);
        if (iNext == string::npos) {
            invalidEntry(line, indexFileName, lineNum);
        }
        string offsetStr = line.substr(iLine, (iNext - iLine));
        iLine = iNext+1;

        // Convert offset to number
        bool isOk = false;
        int offset = Convert::toInt(offsetStr, &isOk);
        if (!isOk) {
            invalidEntry(line, indexFileName, lineNum);
        }
        
        // id
        if ((iLine >= line.size()) || (line[iLine] != '>')) {
            invalidEntry(line, indexFileName, lineNum);
        }
        iLine++;
        iNext = line.find_first_of(ID_TERMINATORS, iLine);
        if (iNext == string::npos) {
            iNext = line.size();
        }
        string id = line.substr(iLine, iNext-iLine);

        // Add an entry, but don't overwrite existing ones with same
        // key
        if (!contains(id)) {
            // Already existed.  Note that add() deleted entry.
            throw IOException("duplicate id in index file: \"" + id + "\"", indexFileName, lineNum);
        }
        insert(id, new IndexEntry(id, offset));
    }

    /*
     * Parse the index file.
     */
    void parseIndexFile(const string& indexFileName) {
        FIOStream indexStream(indexFileName, ios::in);
        string line;
        int lineNum = 0;
        
        while (true) {
            getline(indexStream, line);
            lineNum++;
            if (indexStream.eof()) {
                return;  //EOF
            }
            if (indexStream.fail()) {
                throw IOException("Error reading FASTA index file",
                                  indexFileName, lineNum);
            }
            parseEntry(line, indexFileName, lineNum);
        }
    }

  public:
    
    /** Constructor, parse the index file */
    RecIndex(const string& indexFileName) {
        parseIndexFile(indexFileName);
    }

    /** Get the offset for an id, or -1 if not found */
    int getOffset(const string& seqId) const {
        const IndexEntry* entry = get(seqId);
        if (entry == NULL) {
            return -1;
        } else {
            return entry->fByteOffset;
        }
    }
};

/*
 * Constructor.  Open the fasta file.
 */
Fasta::Fasta(const string& fileName,
             int mode,
             const string& indexFileName):
    fStream(new FIOStream(fileName, ((mode & WRITE) ? ios::out : ios::in))),
    fOwnStream(true),
    fLineNum(0),
    fHaveSequence(false),
    fRecIndex(NULL) {

    if (indexFileName.size() > 0) {
        fRecIndex = new RecIndex(indexFileName);
    }
}

/*
 * Constructor around open file.
 */
Fasta::Fasta(FIOStream* fastaStream,
             int mode,
             int lineNum):
    fStream(fastaStream),
    fOwnStream(false),
    fLineNum(lineNum),
    fHaveSequence(false),
    fRecIndex(NULL) {
}

/*
 * Destructor.
 */
Fasta::~Fasta() {
    for (int idx = 0; idx < (int)fFilters.size(); idx++) {
        delete fFilters[idx];
    }
    if (fRecIndex != NULL) {
        delete fRecIndex;
    }
    if (fOwnStream) {
        delete fStream;
    }
}

/*
 * Parse the id/comment line.
 */
void Fasta::parseId() {
    assert((fLineBuf.size() > 0) && (fLineBuf[0] == '>'));

    // Find the end of the id (which must start with `>')
    int idEnd = fLineBuf.find_first_of(ID_TERMINATORS, 0);
    if (idEnd < 1) {
        idEnd = fLineBuf.size();
    }
    fSeqId = fLineBuf.substr(1, idEnd-1);

    if (fSeqId.size() == 0) {
        fLineBuf.erase();
        throw IOException("Empty sequence id", fStream->getFileName(), fLineNum);
    }

    // Get comment.
    int len = (int)fLineBuf.size();
    idEnd++;
    while ((idEnd < len) && (isspace(fLineBuf[idEnd]))) {
        idEnd++;
    }
    if (idEnd < len) {
        fComment = fLineBuf.substr(idEnd);
    } else {
        fComment = "";
    }
}

/*
 * Read a line into the buffer.
 */
bool Fasta::readLine() {
    getline(*fStream, fLineBuf);
    fLineNum++;
    if (fStream->eof()) {
        fLineBuf.erase();
        return false;
    }
    if (fStream->fail()) {
        fLineBuf.erase();
        throw IOException("Error reading FASTA file ", fStream->getFileName(),
                          fLineNum);
    }
    fLineBuf = StringOps::trimTrailingBlanks(fLineBuf);
    return true;
}

/*
 * Read the next record into the object.
 */
bool Fasta::readNextRec() {
    fHaveSequence = false;

    // Find the record.
    while (true) {
        // Read line if we don't have one in the buffer.
        if (fLineBuf.size() == 0) {
            if (!readLine()) {
                return false;
            }
        }
        if (fLineBuf.size() > 0) {
            if (fLineBuf[0] != '>') {
                string msg("Invalid line in fasta file, expected `>id' line found \"" + fLineBuf + "\"");
                fLineBuf.erase();
                throw IOException(msg, fStream->getFileName(), fLineNum);
            }
            break;  // Got id!
        }
        // reached here, its an empty line
    }

    parseId();

    // Read sequence.
    fData.erase();
    while (true) {
        if (!readLine()) {
            if (fData.size() == 0) {
                goto noSeqFoundError;
            }
            fHaveSequence = true;
            return true;  // Terminated by EOF.
        }
        
        if ((fLineBuf.size() == 0) || (fLineBuf[0] == '>')) {
            if (fData.size() == 0) {
                goto noSeqFoundError;
            }
            fHaveSequence = true;
            return true;  // Terminated by blank line or next sequence
        }

        // Append to sequence
        fData.append(fLineBuf);
    }

    // No sequence found error.
  noSeqFoundError:
    string msg("No sequence found for \"" + fSeqId + "\"");
    fLineBuf.erase();
    throw IOException(msg, fStream->getFileName(), fLineNum);
}

/*
 * Run filters on current sequence.
 */
bool Fasta::runFilters() {
    for (int idx = 0; idx < (int)fFilters.size(); idx++) {
        if (!(fFilters[idx]->filter(fSeqId, fComment, fData))) {
            return false;  // Filter rejected.
        }
    }
    return true; // none rejected
}
/*
 * Read the next available record into the object.
 * Skips past records the filters reject.
 */
bool Fasta::readRec() {
    bool stat;
    while ((stat = readNextRec())) {
        if (runFilters()) {
            break; // Accepted sequence
        }
    }
    return stat;
}

/**
 * Seek to a a record given it's ID.
 */
bool Fasta::readRec(const string& seqId,
                    bool noError) {
    if (fSeqId == seqId) {
        return true; // Already at this sequence
    }
    if (fRecIndex == NULL) {
        throw IOException("seekRec on a fasta file without an index",
                          fStream->getFileName());
    }
    if (fStream->isCompressed()) {
        throw IOException("seekRec on a compressed fasta file",
                          fStream->getFileName());
    }
    int offset = fRecIndex->getOffset(seqId);
    if (offset < 0) {
        if (!noError) {
            throw IOException("sequence id not found in FASTA index: \""
                              + seqId + "\"", fStream->getFileName());
        }
        return false;
    }
    fStream->clear();
    fStream->seekp(offset);
    if (fStream->eof() || fStream->fail()) {
        throw IOException("FASTA seek failed: \"" + seqId + "\"",
                          fStream->getFileName());
    }
    
    // Lets make sure this is a record
    int ch = fStream->get();
    fStream->unget();

    if (ch != '>') {
        throw IOException("FASTA index entry didn't point to on a record: \""
                          + seqId + "\"", fStream->getFileName());
    }

    // Read the actual record, clear any buffered first
    fLineBuf.erase();
    readRec();
    if (fSeqId != seqId) {
        throw IOException("FASTA random read for \""
                          + seqId + "\" returned \"" + fSeqId
                          + "\"", fStream->getFileName());
    }
    return true;
}

/**
 * Write a sequence.
 */
void Fasta::writeRec(const string& id,
                     const string& comment,
                     const string& seqString) {
    *fStream << ">" << id << " " << comment << endl;

    const char* seq = seqString.c_str();
    int idx = 0;
    int charsToWrite = seqString.size();
    while (charsToWrite > 0) {
        int writeLen = ((charsToWrite < MAX_LINE_LENGTH) ? charsToWrite : MAX_LINE_LENGTH);
        fStream->write(seq+idx, writeLen);
        *fStream << endl;
        idx += writeLen;
        charsToWrite -= writeLen;
    }
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * End:
 */

