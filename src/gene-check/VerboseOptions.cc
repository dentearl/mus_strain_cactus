/*
 * FILE: VerboseOptions.cc
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

#include "VerboseOptions.h"
#include "VerboseOption.h"
#include "CmdOptions.h"
#include "FatalError.h"
#include "Convert.h"
#include "Format.h"

/*
 * Verbose command option.
 */
const StringCmdOptionDef
VerboseOptions::OPT_VERBOSE("--verbose", "name - enables verbose printing for `name'", true);

/*
 * Hash table of known options.
 */
StringMap<VerboseOption*>* VerboseOptions::fOptionTable = NULL;

/*
 * singleton instance to force call to destructor.
 */
static VerboseOptions janitor;

/*
 * destructor, only used to cleanup of fOptionTable at exit
 */
VerboseOptions::~VerboseOptions() {
    delete fOptionTable;
    fOptionTable = NULL;
}

/*
 * Method used to register an verbose option object with this class.
 */
void VerboseOptions::define(VerboseOption* verboseOption) {
    if (fOptionTable == NULL) {
        fOptionTable = new VerboseOptionMap();
    }
    if (fOptionTable->contains(verboseOption->getName())) {
        throw FatalError("VerboseOption \"" + verboseOption->getName()
                         + "\" already exists for: "
                         + verboseOption->getHelp());
    }
    fOptionTable->insert(verboseOption->getName(), verboseOption);
}

/**
 * Process a -verbose option name from the command line
 */
bool VerboseOptions::setOption(const string& name,
                               ostream* out) {
    if (fOptionTable == NULL) {
        return false;
    }
    VerboseOption* option = fOptionTable->get(name);
    if (option == NULL) {
        return false;
    }
    option->enable(out);
    return true;
}

/*
 * Print a list of legal options and help messages to the specified file.
 */
void VerboseOptions::printLegalOptions(ostream& out) {
    if (fOptionTable != NULL) {
        for (Generator<StringMap<VerboseOption*>::SuperType> opts(fOptionTable->getEntries()); opts.have(); opts.next()) {
            out << "  " << OPT_VERBOSE.getName() << "=" << opts->second->getName() << endl;
            Format::printLinesIndented(out, opts->second->getHelp(), 4);
        }
    }
}

/*
 * Handle setting a single -verbose option.
 */
void VerboseOptions::processOption(const string& name,
                                   ostream& out) {
    if (!setOption(name, &out)) {
        cerr << "Invalid --verbose value: \"" << name << "\", legal options are:" << endl; 
        printLegalOptions(cerr);
        exit(1);
    }
}

/*
 * Function to process options called by CmdOptions.  
 * FIXME: The explicit relationship between this class and CmdOptions
 * is a little weird, but it gets verbose in everything.
 */
void VerboseOptions::processCmdOptions(CmdOptions& cmdOptions) {
    const StringCmdOptionValues* values = cmdOptions.getStringOptionValues(&OPT_VERBOSE);
    if (values != NULL) {
        for (int i = 0; i < values->size(); i++) {
            processOption((*values)[i]->getValue(), cerr);
        }
    }
}
