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

#ifndef VERBOSE_OPTION_H
#define VERBOSE_OPTION_H

#include <typeinfo>
#include <string>
#include <iostream>
using namespace std;

/**
 * Define a verbose option.  A class-static or global variable of this class
 * is created to define an verbose option and help string.  It will register
 * automatically when constructed.
 */
class VerboseOption {
 private:
    string fName;
    string fHelp;
    ostream* fOutStream;

    /** global node id to prefix message with if >= 0 */
    static int sNodeId;
 public:
    /**
     * Define a verbose option.
     *
     * @param name The name of the option, as will be supplied to 
     *  -verbose.
     * @param help The help message describing the option.  It maybe
     *  multi-line by inserting newlines.
     */
    VerboseOption(const string& name,
                  const string& help);

    /* Set the global node id for messages */
    static void setNodeId(int nodeId) {
        sNodeId = nodeId;
    }

    /**
     * Get the flag indicating if this verbose option is enabled.
     */
    inline bool isOn() const {
        return (fOutStream != NULL);
    }

    /**
     * Output a prefix string for a message with the verbose
     * option name.  Also returns the output stream.
     */
    inline ostream& outPrefix() {
        if (sNodeId >= 0) {
            *fOutStream << "<" << sNodeId << "> ";
        }
        *fOutStream << fName << ": ";
        return *fOutStream;
    }

    /**
     * Get output stream for writting the verbose messages.
     */
    inline ostream& getOut() const {
        return *fOutStream;
    }

    /**
     * Get the option name
     */
    const string& getName() const {
        return fName;
    }

    /**
     * Get the option help message.
     */
    const string& getHelp() const {
        return fHelp;
    }

    /**
     * Enable the option. Used by <code>VerboseOptions</code> only.
     */
    void enable(ostream* out) {
        fOutStream = out;
    }
};
#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
