
#include "CmdOptions.h"
#include "CmdOptionsException.h"
#include "VerboseOptions.h"
#include "Convert.h"
#include "StringOps.h"
#include "FIOStream.h"
#include "FileOps.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

// FIXME: It would be cleaner if each of the option types was its
// own class instead of all of the switch statements in here.

// FIXME: support callback to validate options

/*
 * -read standard option.
 */
const StringCmdOptionDef
CmdOptions::OPT_READ("--read", "optsfile - read command line options from optsfile", true);

/*
 * -verbose command standard option
 */
static VerboseOption verboseCommand(
  "command",
  "print the command lines after all options files have been read");


/*
 * Constructor.
 */
CmdOptions::CmdOptions(int minArgs,
                       int maxArgs,
                       const string& usageMsg,
                       const CmdOptionDef* defs[]):
    fUsageMsg(usageMsg),
    fMinNumArgs(minArgs),
    fMaxNumArgs(maxArgs),
    fCurrentReadLevel(0) {

    setLineBuf();

    // --read and --verbose are always available.
    addOptionDef(&OPT_READ);
    addOptionDef(&VerboseOptions::OPT_VERBOSE);

    if (defs != NULL) {
        addOptionDefs(defs);
    }
}

/*
 * print usage for one option.
 */
void CmdOptions::optionUsage(const CmdOptionDef* def) const {
    StringVector lines = StringVector::split(string(def->getHelp()), '\n');
    cerr << "    " << def->getName();
    cerr << (def->requiresValue() ? "=" : " ");
    for (int i = 0; i < lines.size(); i++) {
        if (i > 0) {
            cerr << "      ";
        }
        cerr << lines[i] << endl;
    }
}

/*
 * Output usage message and exit.
 */
void CmdOptions::usage(const string &msg,
                       bool printOpts) const {
    cerr << "Error: " << msg << endl;
    cerr << fCmdName << " " << fUsageMsg << endl;
    if (printOpts) {
        cerr << "Valid options are:" << endl;
        for (int idx = 0; idx < (int)fDefs.size(); idx++) {
            optionUsage(fDefs[idx]);
        }
    }
    exit(1);
}

/*
 * Add an option.
 */
void CmdOptions::addOptionDef(const CmdOptionDef* def) {
    fDefs.push_back(def);
}

/*
 * Add a list of option.
 */
void CmdOptions::addOptionDefs(const CmdOptionDef* defs[]) {
    for (int idx = 0; defs[idx] != NULL; idx++) {
        addOptionDef(defs[idx]);
    }
}

/*
 * Find an optionDef by name , return NULL if not found.  The leading --
 * should be included.
 */
const CmdOptionDef* CmdOptions::findOptionDef(const string& optName) const {
    int nopts = fDefs.size();
    for (int idx = 0; idx < nopts; idx++) {
        if (fDefs[idx]->getName() == optName) {
            return fDefs[idx];
        }
    }
    return NULL;
}

/*
 * Get an option def, error if not found.
 */
const CmdOptionDef* CmdOptions::getOptionDef(const string& optName,
                                             const string& srcFile) const {
    const CmdOptionDef* def = findOptionDef(optName);
    if (def == NULL) {
        string msg = "Option \"" + optName + "\" not valid";
        if (srcFile != "") {
            msg += ", found in " + srcFile;
        }
        usage(msg);
    }
    return def;
}

/**
 * Get option value array, or null if it doesn't exist.
 */
CmdOptionValues* CmdOptions::getValues(const CmdOptionDef* def) {
    DefValueMap::iterator valueIter = fValues.find(def);
    if (valueIter == fValues.end()) {
        return NULL;
    } else {
        return &(valueIter->second);
    }
}

/**
 * Get option value array, or null if it doesn't exist.
 */
const CmdOptionValues* CmdOptions::getValues(const CmdOptionDef* def) const {
    DefValueMap::const_iterator valueIter = fValues.find(def);
    if (valueIter == fValues.end()) {
        return NULL;
    } else {
        return &(valueIter->second);
    }
}

/**
 * Get option value array, or empty if it doesn't exist.
 */
const CmdOptionValues* CmdOptions::getValuesEmpty(const CmdOptionDef* def) const {
    static const CmdOptionValues empty;
    const CmdOptionValues* values = getValues(def);
    return (values == NULL) ? &empty : values;
}

/**
 * Get option value array, creating if needed.
 */
CmdOptionValues* CmdOptions::obtainValues(const CmdOptionDef* def) {
    CmdOptionValues* values = getValues(def);
    if (values == NULL) {
        fValues.insert(DefValuePair(def, CmdOptionValues()));
        values = getValues(def);
    }
    return values;
}

/*
 * Parse an option
 */
void CmdOptions::parseOption(const string& arg,
                             const string& srcFile) {
    // split at first =;
    string optName, optValue;
    size_t eqIdx = arg.find('=');
    if (eqIdx == string::npos) {
        optName = arg;  // no value
    } else {
        optName = arg.substr(0, eqIdx);
        optValue = arg.substr(eqIdx+1);
    }
    const CmdOptionDef* def = getOptionDef(optName, srcFile); // fails if unknown
    if ((eqIdx == string::npos) && def->requiresValue()) {
        usage("Option \"" + optName + "\" requires a value");
    }
    const CmdOptionValue* value = def->parse(optValue, srcFile);
    CmdOptionValues* values = obtainValues(def);
    if ((values->size() > 0) && !def->getMultipleAllowed()) {
        usage("Option \"" + def->getName() + "\" specified multiple times, only one occurrence of this options is allowed");
    }
    values->push_back(value);
    
    // Handle --read
    if (def == &OPT_READ) {
        parseOptionFile(dynamic_cast<const StringCmdOptionValue*>(value)->getValue(), srcFile);
    }
}

/*
 * Parse an option file.
 */
void CmdOptions::parseOptionFile(const string& srcFile,
                                 const string& includingFile) {
    fCurrentReadLevel++;
    if (fCurrentReadLevel > MAX_READ_NEST) {
        cerr << "Error: --read nest exceeds max of " << MAX_READ_NEST << ", probable -read loop" << endl;
        exit(1);
    }

    // Construct file name relative to including file and save name
    // in storage pool.
    string realFileName;
    if (includingFile.size() == 0) {
        // absolute path or relative to cwd
        realFileName = srcFile;
    } else {
        realFileName = FileOps::relativePath(FileOps::dir(includingFile),
                                             srcFile);
    }

    FIOStream in(realFileName);
    if (in.fail()) {
        cerr << "Error: can't open command file: " << realFileName << endl;
        exit(1);
    }

    // read and parse lines
    string line;
    while (true) {
        getline(in, line);
        if (in.eof()) {
            break;
        }
        line = StringOps::trimBlanks(line);
        if (!((line.size() == 0) || (line[0] == '#'))) {
            // Not a comment or blank.
            parseOption(line, realFileName);
        }
    }
    fCurrentReadLevel--;
}

/**
 * parse options.
 */
int CmdOptions::parseOptions(int argc,
                             int argi,
                             const char* const argv[]) {
    // Parse the optional arguments.
    while ((argi < argc) && (argv[argi][0] == '-')) {
        if (strcmp(argv[argi], "--") == 0) {
            argi++;
            break;  // Explict end of args
        }
        if (argv[argi][1] != '-') {
            usage("option starts with a single `-', options must start with `--': " + string(argv[argi]));
        }
        parseOption(string(argv[argi]), "");
        argi++;
    }

    return argi;
}

/*
 * Parse the command line
 */
void CmdOptions::parse(int argc,
                       const char* const argv[]) {
    // Parse the optional arguments.
    int argi = 1;
    fCmdName = argv[0];
    argi = parseOptions(argc, argi, argv);

    // Handle positional arguments.
    for (; argi < argc; argi++) {
        fPositionalArgs.push_back(string(argv[argi]));
    }

    if ((fMinNumArgs >= 0) && (fPositionalArgs.size() < fMinNumArgs)) {
        usage("too few arguments");
    }
    if ((fMaxNumArgs >= 0) && (fPositionalArgs.size() > fMaxNumArgs)) {
        usage("too many arguments");
    }

    // Special-case --verbose
    VerboseOptions::processCmdOptions(*this);

    if (verboseCommand.isOn()) {
        printCmd(verboseCommand.outPrefix());
    }
}

/*
 * Print an option and its values.
 */
void CmdOptions::printOption(ostream& out,
                             const CmdOptionDef* def) const {
    // Output one or more instances the option
    const CmdOptionValues* values = getValues(def);
    if (values != NULL) {
        for (unsigned iOpt = 0; iOpt < values->size(); iOpt++) {
            out << " " << def->getName() << '=' << (*values)[iOpt]->toString();
        }
    }
}

/*
 * Print the resulting command after reading of option files.
 */
void CmdOptions::printCmd(ostream& out) const {
    out << fCmdName;

    for (unsigned iOpt = 0; iOpt < fDefs.size(); iOpt++) {
        printOption(out, fDefs[iOpt]);
    }

    for (unsigned iArg = 0; iArg < fPositionalArgs.size(); iArg++) {
        out << " " << fPositionalArgs[iArg];
    }
    out << endl;
}

/**
 * Determine if an option is a defined legal option.
 */
bool CmdOptions::defined(const CmdOptionDef* def) const {
    for (int idx = 0; idx < fDefs.size(); idx++) {
        if (fDefs[idx] == def) {
            return true;
        }
    }
    return false;
}

/** lookup a bool option value, defaults to false */
bool CmdOptions::getBoolValue(const BoolCmdOptionDef* def) const {
    const BoolCmdOptionValues* values = reinterpret_cast<const BoolCmdOptionValues*>(getValues(def));
    if (values == NULL) {
        return false;
    } else {
        assert(values->size() == 1);
        return (*values)[0]->getValue();
    }
}

/** lookup int option value */
int CmdOptions::getIntValue(const IntCmdOptionDef* def,
                            int defaultValue) const {
    const IntCmdOptionValues* values = reinterpret_cast<const IntCmdOptionValues*>(getValues(def));
    if (values == NULL) {
        return defaultValue;
    } else {
        assert(values->size() == 1);
        return (*values)[0]->getValue();
    }
}

/** lookup a real option value */
double CmdOptions::getRealValue(const RealCmdOptionDef* def,
                                double defaultValue) const {
    const RealCmdOptionValues* values = reinterpret_cast<const RealCmdOptionValues*>(getValues(def));
    if (values == NULL) {
        return defaultValue;
    } else {
        assert(values->size() == 1);
        return (*values)[0]->getValue();
    }
}

/** get a single-valued string option value or empty strign */
const string& CmdOptions::getStringValue(const StringCmdOptionDef* def,
                                         const string& defaultValue) const {
    const StringCmdOptionValues* values = reinterpret_cast<const StringCmdOptionValues*>(getValues(def));
    if (values == NULL) {
        return defaultValue;
    } else {
        return (*values)[0]->getValue();
    }
}
    
/** get a single-valued string option value or empty string */
const string& CmdOptions::getStringValue(const StringCmdOptionDef* def) const {
    static string empty;
    return getStringValue(def, empty);
}
    
/** get a single-valued string option value as a relative file path  */
string CmdOptions::getRelFilePathValue(const StringCmdOptionDef* def) const {
    static string empty;
    const StringCmdOptionValues* values = reinterpret_cast<const StringCmdOptionValues*>(getValues(def));
    if (values == NULL) {
        return empty;
    } else {
        return (*values)[0]->getRelFilePath();
    }
}
    
/** get a string option value or null */
const StringCmdOptionValue* CmdOptions::getStringOptionValue(const StringCmdOptionDef* def) const {
    const StringCmdOptionValues* values = reinterpret_cast<const StringCmdOptionValues*>(getValues(def));
    if (values == NULL) {
        return NULL;
    } else {
        return (*values)[0];
    }
}
   
/** get a string option values or empty list */
const StringCmdOptionValues* CmdOptions::getStringOptionValues(const StringCmdOptionDef* def) const {
    return reinterpret_cast<const StringCmdOptionValues*>(getValuesEmpty(def));
}
   
/** get a string vector option value or NULL */
const VectorCmdOptionValue* CmdOptions::getVectorOptionValue(const VectorCmdOptionDef* def) const {
    const VectorCmdOptionValues* values = reinterpret_cast<const VectorCmdOptionValues*>(getValues(def));
    if (values == NULL) {
        return NULL;
    } else {
        return (*values)[0];
    }
}
   
/** get a string option values or empty list */
const VectorCmdOptionValues* CmdOptions::getVectorOptionValues(const VectorCmdOptionDef* def) const {
    return reinterpret_cast<const VectorCmdOptionValues*>(getValuesEmpty(def));
}

/**
 * Utility to set stdout & stderr to line buffered.
 */
void CmdOptions::setLineBuf() {
#if (__GNUC__ > 2)
    // FIXME: any way to force line buffered??
    // FIXME:ios::sync_with_stdio(false);
    // streambuf.linebuffered(true);
#else
    // FIXME: really don't want this if data going to stdout
    cout.rdbuf()->linebuffered(true);
    cerr.rdbuf()->linebuffered(true);
#endif

    setlinebuf(stdout);
    setlinebuf(stderr);
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

