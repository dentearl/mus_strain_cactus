/*
 * FILE: VerboseOption.h
 * AUTHOR: Mark Diekhans
 * CREATE DATE: 
 * PROJECT: G-Known
 * DESCRIPTION: Define a verbose option.
 * VERSION: $Revision$
 *
 * Copyright 1998-1999, The Regents of the University of California
 *
 * Departments of Computer Engineering and Computer Science
 * Jack Baskin School of Engineering
 * University of California, Santa Cruz, CA 95064
 */

#include "VerboseOption.h"
#include "VerboseOptions.h"


/** global node id to prefix message with if >= 0 */
int VerboseOption::sNodeId = -1;

/**
 * Define a verbose option.
 */
VerboseOption::VerboseOption(const string& name,
                             const string& help) {
    fName = name;
    fHelp = help;
    fOutStream = NULL;
    VerboseOptions::define(this);
}
