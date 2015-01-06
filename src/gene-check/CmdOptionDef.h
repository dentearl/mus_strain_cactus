#ifndef CmdOptionDef_h
#define CmdOptionDef_h

#include <typeinfo>
#include <string>
#include "StringVector.h"
using namespace std;
class CmdOptionValue;

/**
 * @name CmdOptionDef
 * Base class used to define options.
 */
class CmdOptionDef {
  private:
    /**
     * Option name, including `--'.
     */
    const string fName;

    /**
     * Help text on option.  Should include arguments and a `-' separator
     * between argument and text.
     */
    const string fHelp;

    /**
     * Allow multiple occurrences of the option.
     */
    const bool fMultipleAllowed;

public:
    /** Constructor */
    CmdOptionDef(const string& name,
                 const string& help,
                 bool multipleAllowed);

    /** Get the option name, including `--' */
    const string& getName() const {
        return fName;
    }

    /** get help string, without argument name. */
    const string& getHelp() const {
        return fHelp;
    }

    /** are multiple occurrences of the option allowed? */
    bool getMultipleAllowed() const {
        return fMultipleAllowed;
    }

    /* must this have a value? */
    virtual bool requiresValue() const {
        return true;  // most do
    }

    /** parse the option value */
    virtual const CmdOptionValue* parse(const string& strValue,
                                        const string& specifyingFile) const = 0;
};

/**
 * A bool command option.
 */
class BoolCmdOptionDef: public CmdOptionDef {
    public:
    /** Constructor */
    BoolCmdOptionDef(const string& name,
                     const string& help):
        CmdOptionDef(name, help, false) {
    }

    /* must this have a value? */
    virtual bool requiresValue() const {
        return false;
    }
    /** parse the option value. strValue may be empty for bool */
    virtual const CmdOptionValue* parse(const string& strValue,
                                        const string& specifyingFile) const;
};

/**
 * An int command option.
 */
class IntCmdOptionDef: public CmdOptionDef {
    public:
    /** Constructor */
    IntCmdOptionDef(const string& name,
                    const string& help):
        CmdOptionDef(name, help, false) {
    }

    /** parse the option value */
    virtual const CmdOptionValue* parse(const string& strValue,
                                        const string& specifyingFile) const;
};

/**
 * An real (double) command option.
 */
class RealCmdOptionDef: public CmdOptionDef {
    public:
    /** Constructor */
    RealCmdOptionDef(const string& name,
                     const string& help):
        CmdOptionDef(name, help, false) {
    }

    /** parse the option value */
    virtual const CmdOptionValue* parse(const string& strValue,
                                        const string& specifyingFile) const;
};

/**
 * An string command option.
 */
class StringCmdOptionDef: public CmdOptionDef {
    public:
    /** Constructor */
    StringCmdOptionDef(const string& name,
                       const string& help,
                       bool multipleAllowed=false):
        CmdOptionDef(name, help, multipleAllowed) {
    }

    /** parse the option value */
    virtual const CmdOptionValue* parse(const string& strValue,
                                        const string& specifyingFile) const;
};

/**
 * An Vector command option.
 */
class VectorCmdOptionDef: public CmdOptionDef {
    private:
    /**
     * Number of values for a single instance of the option.
     */
    const int fNumValues;

    /** separator for values */
    const char fSeparator;

    public:
    /** 
     * Constructor
     * @param numValues required number of values, -1 to not check number.
     * @param separator separator for values
     */
    VectorCmdOptionDef(const string& name,
                       const string& help,
                       int numValues,
                       bool multipleAllowed=false,
                       char separator=','):
        CmdOptionDef(name, help, multipleAllowed),
        fNumValues(numValues),
        fSeparator(separator) {
    }

    /** number of values */
    int getNumValues() const {
        return fNumValues;
    }

    /* separator */
    char getSeparator() const {
        return fSeparator;
    }

    /** parse the option value */
    virtual const CmdOptionValue* parse(const string& strValue,
                                        const string& specifyingFile) const;
};

/**
 * base class for a value of a parsed command option.
 */
class CmdOptionValue {
    private:
    const CmdOptionDef* const fDef;
    const string fSpecifyingFile;

    public:
    CmdOptionValue(const CmdOptionDef* def,
                   const string& specifyingFile):
        fDef(def),
        fSpecifyingFile(specifyingFile) {
    }
    virtual ~CmdOptionValue() {
    }

    /* get the definition */
    const CmdOptionDef* getDef() const {
        return fDef;
    }

    /* get the source file */
    const string& getSpecifyingFile() const {
        return fSpecifyingFile;
    }

    /* get value as a string */
    virtual string toString() const = 0;
};

/*
 * Vectors of generic  values.
 */
typedef vector<const CmdOptionValue*> CmdOptionValues;

/**
 * A bool command option value.
 */
class BoolCmdOptionValue: public CmdOptionValue {
    private:
    bool fValue;

    public:
    /** Constructor */
    BoolCmdOptionValue(const BoolCmdOptionDef* def,
                       const string& specifyingFile,
                       bool value):
        CmdOptionValue(def, specifyingFile),
        fValue(value) {
    }

    /** value accessor */
    bool getValue() const {
        return fValue;
    }

    /* get value as a string */
    virtual string toString() const;
};

/*
 * Vectors of bool values.
 */
typedef vector<const BoolCmdOptionValue*> BoolCmdOptionValues;

/**
 * An int command option value.
 */
class IntCmdOptionValue: public CmdOptionValue {
    private:
    int fValue;

    public:
    /** Constructor */
    IntCmdOptionValue(const IntCmdOptionDef* def,
                      const string& specifyingFile,
                      int value):
        CmdOptionValue(def, specifyingFile),
        fValue(value) {
    }

    /** value accessor */
    int getValue() const {
        return fValue;
    }

    /* get value as a string */
    virtual string toString() const;
};

/*
 * Vectors of int  values.
 */
typedef vector<const IntCmdOptionValue*> IntCmdOptionValues;

/**
 * An real (double) command option value.
 */
class RealCmdOptionValue: public CmdOptionValue {
    private:
    double fValue;

    public:
    /** Constructor */
    RealCmdOptionValue(const RealCmdOptionDef* def,
                       const string& specifyingFile,
                       double value):
        CmdOptionValue(def, specifyingFile),
        fValue(value) {
    }

    /** value accessor */
    double getValue() const {
        return fValue;
    }

    /* get value as a string */
    virtual string toString() const;
};

/*
 * Vectors of real values.
 */
typedef vector<const RealCmdOptionValue*> RealCmdOptionValues;


/**
 * An string command option value.
 */
class StringCmdOptionValue: public CmdOptionValue {
    private:
    string fValue;

    public:
    /** Constructor */
    StringCmdOptionValue(const StringCmdOptionDef* def,
                         const string& specifyingFile,
                         const string& value):
        CmdOptionValue(def, specifyingFile),
        fValue(value) {
    }


    /** value accessor */
    const string& getValue() const {
        return fValue;
    }

    /** get the value as relative file path to the specifying file. */
    string getRelFilePath() const;

    /* get value as a string */
    virtual string toString() const {
        return fValue;
    }
};

/*
 * Vectors of string values.
 */
typedef vector<const StringCmdOptionValue*> StringCmdOptionValues;

/**
 * An Vector command option value.
 */
class VectorCmdOptionValue: public CmdOptionValue {
    private:
    StringVector fValue;

    public:
    /** 
     * Constructor
     */
    VectorCmdOptionValue(const VectorCmdOptionDef* def,
                         const string& specifyingFile,
                         const StringVector& value):
        CmdOptionValue(def, specifyingFile),
        fValue(value) {
    }

    /** value accessor */
    const StringVector& getValue() const {
        return fValue;
    }

    /** get the number of elements parsed from the value */
    int getNumElements() const {
        return fValue.size();
    }

    /** get an element parsed from the value */
    const string& getElement(int iValue) const {
        return fValue[iValue];
    }

    /** get an element parsed from the value */
    const string& getString(int iValue) const {
        return fValue[iValue];
    }

    /** get an element parsed from the value as a bool */
    bool getBool(int iValue) const;

    /** get an element parsed from the value as an int */
    int getInt(int iValue) const;

    /** get an element parsed from the value as a real */
    double getReal(int iValue) const;

    /** get an element parsed from the value as relative file path to the
     * specifying file. */
    string getRelFilePath(int iValue) const;

    /* get value as a string */
    virtual string toString() const;
};

/*
 * Vectors of vector values.
 */
typedef vector<const VectorCmdOptionValue*> VectorCmdOptionValues;


#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

