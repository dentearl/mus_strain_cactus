#ifndef Generator_h
#define Generator_h

/* 
 * Generator over an STL container.  This is an iterator that carries around end.
 *
 * Note that various methods were tried to define derived Generator,  typedefs
 * of concrete class seems the most portable:
 *  typedef Generator<StringMap<Xxx*>::SuperType> XxxMapGenerator;
 */
template<class OBJ>
class Generator {
    private:
    typename OBJ::iterator fStart; // where started, so it can be reset
    typename OBJ::iterator fIter;
    typename OBJ::iterator fEnd;

    public:
    /* constructor */
    Generator(typename OBJ::iterator iter,
              typename OBJ::iterator end):
        fStart(iter),
        fIter(iter),
        fEnd(end) {
    }

    /* constructor for null iterator */
    Generator() {
    }

    /** get start */
    typename OBJ::iterator getStart() const {
        return fStart;
    }

    /** get current iter */
    typename OBJ::iterator getIter() const {
        return fIter;
    }

    /** get end */
    typename OBJ::iterator getEnd() const {
        return fEnd;
    }

    /** Check if the is an element to access */
    inline bool have() const {
        return (fIter != fEnd);
    }

    /** Advance to the next item, return false if no more */
    inline bool next() {
        fIter++;
        return have();
    }

    /** pointer dereference of current element */
    inline typename OBJ::iterator operator->() {
        return fIter;
    }
    
    /** get pointer current element */
    inline typename OBJ::iterator operator*() {
        return fIter;
    }

    /* reset to the start */
    void reset() {
        fIter = fStart;
    }
};


/* 
 * const Generator over an STL container.  This is an iterator that carries around end.
 *
 * Note that various methods were tried to define derived Generator,  typedefs
 * of concrete class seems the most portable:
 *  typedef ConstGenerator<StringMap<Xxx*>::SuperType> XxxMapConstGenerator;
 */
template<class OBJ>
class ConstGenerator {
    private:
    typename OBJ::const_iterator fStart; // where started, so it can be reset
    typename OBJ::const_iterator fIter;
    typename OBJ::const_iterator fEnd;

    public:
    ConstGenerator(typename OBJ::const_iterator iter,
                   typename OBJ::const_iterator end):
        fStart(iter),
        fIter(iter),
        fEnd(end) {
    }

    /* constructor for null iterator */
    ConstGenerator():
        fStart(NULL),
        fIter(NULL),
        fEnd(NULL) {
    }

    /* initialize from non-const */
    ConstGenerator(const Generator<OBJ>& src):
        fStart(src.getStart()),
        fIter(src.getIter()),
        fEnd(src.getEnd()) {
    }

    /** get start */
    typename OBJ::iterator getStart() const {
        return fStart;
    }

    /** get current iter */
    typename OBJ::iterator getIter() const {
        return fIter;
    }

    /** get end */
    typename OBJ::iterator getEnd() const {
        return fEnd;
    }

    /** Check if the is an element to access */
    inline bool have() const {
        return (fIter != fEnd);
    }

    /** Advance to the next item, return false if no more */
    inline bool next() {
        fIter++;
        return have();
    }

    /** pointer dereference of current element */
    inline typename OBJ::const_iterator operator->() {
        return fIter;
    }
    
    /** get pointer current element */
    inline typename OBJ::const_iterator operator*() {
        return fIter;
    }

    /* reset to the start */
    void reset() {
        fIter = fStart;
    }
};
#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

