#ifndef CmdOptions_h
#define CmdOptions_h

#include <typeinfo>
#include <map>
#include "CmdOptionDef.h"

/**
 * Command options parser.  This parse options and values from a command
 * line argument vector.  This does not follow the getopt conventions.
 * The following rules apply:
 *
 * <UL>
 * <LI> Options start with `--', e.g. <CODE>--verbose</CODE> and
 *      can not be grouped in a single `-'.
 * <LI> An option is separated from a value with an `=', only bool options
 *      do not have an argument.
 * <LI> An argument of `--' terminates the options.
 * <LI> A given option can be specified as only being allowed once, or
 *      allowed multiple times.
 * <LI> String options
 * <LI> A standard option <CODE>--read</CODE> will read options from
 *      a file, one option plus arguments per-line.  Blank lines and
 *      lines were the first non-whitespace character is `#' are ignored.
 *      The <CODE>-=read</CODE> option maybe nested.  Relative files
 *      names for nested reads read the file relative to the including
 *      file.  Command that read files, use file names relative
 *      to the file contained the option.
 * </UL>
 *
 * A program wishing to use this class defines an constant of
 * type <CODE>CmdOptionDef</CODE> for each option.
 * An array of these objects are passed in when the object is
 * constructed.  The definitions should then be used to get the
 * values of the options.
 *
 * @author Mark Diekhans &lt;markd@cse.ucsc.edu&gt;
 */
class CmdOptions {
  private:
    // Options
    vector<const CmdOptionDef*> fDefs;
    
    typedef map<const CmdOptionDef*, CmdOptionValues> DefValueMap;
    typedef pair<const CmdOptionDef*, CmdOptionValues> DefValuePair;

    // list of parsed values, indexed by def pointer 
    DefValueMap fValues;

    // Usage message.
    const string fUsageMsg;

    // Min/max number of positional arguments or -1 if unspecified.
    int fMinNumArgs;
    int fMaxNumArgs;

    // Command name
    string fCmdName;

    // positional arguments
    StringVector fPositionalArgs;

    // Maximun read level and current level.
    static const int MAX_READ_NEST = 64;
    int fCurrentReadLevel;

    // --read standard option
    static const StringCmdOptionDef OPT_READ;

    /*
     * internal functions
     */
    void optionUsage(const CmdOptionDef* def) const;
    const CmdOptionDef* findOptionDef(const string& optName) const;
    const CmdOptionDef* getOptionDef(const string& optName,
                                     const string& srcFile="") const;
    CmdOptionValues* getValues(const CmdOptionDef* def);
    const CmdOptionValues* getValues(const CmdOptionDef* def) const;
    const CmdOptionValues* getValuesEmpty(const CmdOptionDef* def) const;
    CmdOptionValues* obtainValues(const CmdOptionDef* def);
    void parseOption(const string& arg,
                     const string& srcFile);
    void parseOptionFile(const string& srcFile,
                         const string& includingFile);
    int parseOptions(int argc,
                     int argi,
                     const char* const argv[]);
    void printOption(ostream& out,
                     const CmdOptionDef* def) const;
public:
    /**
     * Constructor, specify command line. 
     *
     * @param minArgs Minimum number of positional arguments, -1 if
     *  unspecified.
     * @param maxArgs Maximum number of positional arguments, -1 if
     *  unspecified.
     * @param usageMsg Message to print that shows the expected arguments.
     *  This should not include the command name.
     * @param cmdOptions An array of pointers to option definitions that is
     *  terminated by NULL pointer.
     */
    CmdOptions(int minArgs,
               int maxArgs,
               const string& usageMsg,
               const CmdOptionDef* defs[] = NULL);

    /**
     * Destructor.
     */
    ~CmdOptions() {
    }

    /*
     * Add an option.
     */
    void addOptionDef(const CmdOptionDef* def);

    /*
     * Add an list of options.
     */
    void addOptionDefs(const CmdOptionDef* cmdOptionDefs[]);

    /**
     * Output usage message and exit.
     */
    void usage(const string &msg,
               bool printOpts = true) const;

    /**
     * Parse the options.
     *
     * @param argc Argument count.
     * @param argv Argument vector.
     */
    void parse(int argc,
               const char* const argv[]);

    /*
     * Print the resulting command after reading of option files.
     */
    void printCmd(ostream& out) const;

    /**
     * Get the number of defined options.
     */
    int getNumCmdOptionDefs() const {
        return (int)fDefs.size();
    }

    /**
     * Get an option definition.
     */
    const CmdOptionDef* getCmdOptionDef(int idx) const {
        return fDefs[idx];
    }

    /**
     * Determine if an option is a defined legal option.
     */
    bool defined(const CmdOptionDef* def) const;

    /**
     * Determine if an option was specified.
     */
    bool specified(const CmdOptionDef* def) const {
        return (getValues(def) != NULL);
    }

    /** lookup a bool option value, defaults to false */
    bool getBoolValue(const BoolCmdOptionDef* def) const;

    /** lookup int option value */
    int getIntValue(const IntCmdOptionDef* def,
                    int defaultValue = 0) const;

    /** lookup a real option value */
    double getRealValue(const RealCmdOptionDef* def,
                        double defaultValue = 0.0) const;

    /** get a single-valued string option value, or default */
    const string& getStringValue(const StringCmdOptionDef* def,
                                 const string& defaultValue) const;
    
    /** get a single-valued string option value, or empty string */
    const string& getStringValue(const StringCmdOptionDef* def) const;
    
    /** get a single-valued string option as a file path */
    string getRelFilePathValue(const StringCmdOptionDef* def) const;
    
    /** get a string option value or null */
    const StringCmdOptionValue* getStringOptionValue(const StringCmdOptionDef* def) const;
   
    /** get a string option values or empty list */
    const StringCmdOptionValues* getStringOptionValues(const StringCmdOptionDef* def) const;
   
    /** get a string vector option value or NULL */
    const VectorCmdOptionValue* getVectorOptionValue(const VectorCmdOptionDef* def) const;
   
    /** get a string option values or empty list */
    const VectorCmdOptionValues* getVectorOptionValues(const VectorCmdOptionDef* def) const;
   
    /**
     * Get the number of positional arguments.
     */
    int getNumPositionalArgs() const {
        return (int)fPositionalArgs.size();
    }

    /**
     * Get a positional argument.
     */
    const string& getPositionalArg(int idx) const {
        return fPositionalArgs[idx];
    }

    /**
     * Get all positional argument.
     */
    const StringVector& getPositionalArgs() const {
        return fPositionalArgs;
    }

    /**
     * Utility to set stdout & stderr to line buffered. Called automatically
     * by constructor, but might be needed explictly.
     */
    static void setLineBuf();
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-off: 4
 * End:
 */

