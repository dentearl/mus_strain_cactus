/*
 * FILE: VerboseOptions.h
 * AUTHOR: Mark Diekhans
 * CREATE DATE: 
 * PROJECT: G-Known
 * DESCRIPTION: Management of verbose options for programs.
 * VERSION: $Revision$
 *
 * Copyright 1998-1999, The Regents of the University of California
 *
 * Departments of Computer Engineering and Computer Science
 * Jack Baskin School of Engineering
 * University of California, Santa Cruz, CA 95064
 */

#ifndef VERBOSE_OPTIONS_H
#define VERBOSE_OPTIONS_H

#include <typeinfo>
#include <string>
#include "CmdOptions.h"
#include "VerboseOption.h"
#include "StringMap.h"

/**
 * Management of verbose options for programs.  Software that wishs to define
 * a boolean verbose option defines a class-static or global variable of class
 * <code>VerboseOption</code>.  If this module is linked into a program, the
 * automatication instantation of the global variable registers the option
 * with this class.  When options are parsed, this class will set the
 * <code>on</code> field on <code>VerboseObject</code> objects.  This can then
 * be checked by code to determine if verbose output should be created.  A
 * pointer to output stream to write to is also stored the objects.  This will
 * allow collection of verbose output to different files if that is ever
 * needed.
 */
class VerboseOptions {
    // WARNING: Don't go creating static object fields here, as static
    // initialzation is used to register with this class and there is no way
    // to know the order that constructors are called.  Instead, we have a
    // pointer to the hash table that is explictly set if its NULL.

 private:
    /**
     * Hash table of known options.  This must be a pointer, see warning
     * above.
     */
    typedef StringMap<VerboseOption*> VerboseOptionMap;
    static VerboseOptionMap* fOptionTable;

    /*
     * Handle setting a single -verbose option.
     */
    static void processOption(const string& name,
                              ostream& out);

 public:
    /* destructor, only used to force cleanup of fOptionTable at exit */
    ~VerboseOptions();

    /**
     * Verbose command option that is automatically added by
     * CmdOptions facility.
     */
    static const StringCmdOptionDef OPT_VERBOSE;

    /**
     * Method used to register an verbose option object with this class.
     * Called by the VerboseObject constructor.
     */
    static void define(VerboseOption* verboseOption);

    /**
     * Process a -verbose option name from the command line
     *
     * @param name The name of the verbose option
     * @param out The output stream to use for verbose output.
     * @return true if the option is legal, false if its not.
     */
    static bool setOption(const string& name,
                          ostream* out);

    /**
     * Print a list of legal options and help messages to the
     * specified file.
     */
    static void printLegalOptions(ostream& out);

    /**
     * Function to process options called by CmdOptions.  
     */
    static void processCmdOptions(CmdOptions& cmdOptions);
};
#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
 
