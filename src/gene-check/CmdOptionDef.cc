#include "CmdOptionDef.h"
#include "CmdOptionsException.h"
#include "StringOps.h"
#include "FileOps.h"
#include "Convert.h"
#include "CmdOptionsException.h"

/** Constructor */
CmdOptionDef::CmdOptionDef(const string& name,
                           const string& help,
                           bool multipleAllowed):
    fName(name),
    fHelp(help),
    fMultipleAllowed(multipleAllowed) {
    if (!StringOps::startsWith("--", name)) {
        throw CmdOptionsException("command definition does not start with `--': " + name);
    }
}

/** parse the option value */
const CmdOptionValue* BoolCmdOptionDef::parse(const string& strValue,
                                              const string& specifyingFile) const {
    bool value;
    if (strValue.size() == 0) {
        value = true;
    } else {
        bool isOk = false;
        value = Convert::toBool(strValue, &isOk);
        if (!isOk) {
            throw CmdOptionsException("invalid value for " + getName() + " \"" + strValue + "\" expected no value or one of: true, on, yes, false, off, no");
        }
    }
    return new BoolCmdOptionValue(this, specifyingFile, value);
}

/** parse the option value */
const CmdOptionValue* IntCmdOptionDef::parse(const string& strValue,
                                             const string& specifyingFile) const {
    bool isOk = false;
    int value = Convert::toInt(strValue, &isOk);
    if (!isOk) {
        throw CmdOptionsException("invalid value for " + getName() + " \"" + strValue + "\" expected an integer");
    }
    return new IntCmdOptionValue(this, specifyingFile, value);
}

/** parse the option value */
const CmdOptionValue* RealCmdOptionDef::parse(const string& strValue,
                                              const string& specifyingFile) const {
    bool isOk = false;
    double value = Convert::toDouble(strValue, &isOk);
    if (!isOk) {
        throw CmdOptionsException("invalid value for " + getName() + " \"" + strValue + "\" expected an real number");
    }
    return new RealCmdOptionValue(this, specifyingFile, value);
}

/** parse the option value */
const CmdOptionValue* StringCmdOptionDef::parse(const string& strValue,
                                                const string& specifyingFile) const {
    return new StringCmdOptionValue(this, specifyingFile, strValue);
}

/** parse the option value */
const CmdOptionValue* VectorCmdOptionDef::parse(const string& strValue,
                                                const string& specifyingFile) const {
    StringVector values = StringVector::split(strValue, fSeparator);
    if ((fNumValues >= 0) && (values.size() != fNumValues)) {
        throw CmdOptionsException("invalid value for " + getName() + " \"" + strValue + "\" expected " + Convert::toString(fNumValues)  + " of `" + Convert::toString(fSeparator) + "' separated values");
    }
    return new VectorCmdOptionValue(this, specifyingFile, values);
}

/* get value as a string */
string BoolCmdOptionValue::toString() const {
    return Convert::toString(fValue);
}

/* get value as a string */
string IntCmdOptionValue::toString() const {
    return Convert::toString(fValue);
}

/* get value as a string */
string RealCmdOptionValue::toString() const {
    return Convert::toString(fValue);
}

/** get the value as relative file path to the specifying file. */
string StringCmdOptionValue::getRelFilePath() const {
    if (getSpecifyingFile().size() > 0) {
        return FileOps::relativePath(FileOps::dir(getSpecifyingFile()), fValue);
    } else {
        return fValue;
    }
}

/** get an element parsed from the value as a bool */
bool VectorCmdOptionValue::getBool(int iValue) const {
    bool isOk = false;
    bool elem = Convert::toBool(fValue[iValue], &isOk);
    if (!isOk) {
        throw CmdOptionsException("invalid value for " + getDef()->getName() + " element " + Convert::toString(iValue) + " \"" + fValue[iValue] + "\" one of: true, on, yes, false, off, no");
    }
    return elem;
}

/** get an element parsed from the value as an int */
int VectorCmdOptionValue::getInt(int iValue) const {
    bool isOk = false;
    int elem = Convert::toInt(fValue[iValue], &isOk);
    if (!isOk) {
        throw CmdOptionsException("invalid value for " + getDef()->getName() + " element " + Convert::toString(iValue) + " \"" + fValue[iValue] + "\" expected an integer");
    }
    return elem;
}

/** get an element parsed from the value as a real */
double VectorCmdOptionValue::getReal(int iValue) const {
    bool isOk = false;
    double elem = Convert::toDouble(fValue[iValue], &isOk);
    if (!isOk) {
        throw CmdOptionsException("invalid value for " + getDef()->getName() + " element " + Convert::toString(iValue) + " \"" + fValue[iValue] + "\" expected an real number");
    }
    return elem;
}

/** get the value as relative file path to the specifying file. */
string VectorCmdOptionValue::getRelFilePath(int iValue) const {
    if (getSpecifyingFile().size() > 0) {
        return FileOps::relativePath(FileOps::dir(getSpecifyingFile()), getElement(iValue));
    } else {
        return getElement(iValue);
    }
}

/* get value as a string */
string VectorCmdOptionValue::toString() const {
    return fValue.join(dynamic_cast<const VectorCmdOptionDef*>(getDef())->getSeparator());
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

