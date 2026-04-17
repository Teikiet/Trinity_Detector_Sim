/*
 * vbfCheck.cpp -- verifies the checksum in a VBF file
 * by Filip Pizlo, 2004
 */

#include "VBF/VBankFileReader.h"
#include "VBF/Words.h"

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

static void usage() {
    cerr<<"Usage: vbfCheck <filename> [<expected checksum>]"<<endl;
    cerr<<"Description:"<<endl;
    cerr<<"Calculates a checksum for the file (using the definition of the checksum in"<<endl;
    cerr<<"the VBF specification).  If the <expected checksum> argument is specified,"<<endl;
    cerr<<"the calculated checksum is compared against <expected checksum>.  Otherwise,"<<endl;
    cerr<<"the calculated checksum is compared against the checksum field in the file."<<endl;
    cerr<<"If the checksums match, vbfCheck exits quietly.  If checksum verification"<<endl;
    cerr<<"fails, an error is printed and exit code 1 is returned."<<endl;
    exit(1);
}

int main(int c,char **v) {
    try {
        if (c<2 || c>3) {
            usage();
        }
        
        VBankFileReader reader(v[1],false,true);
        
        if (c==3) {
            uword32 expected_checksum;
            if (sscanf(v[2],"%lu",(long unsigned int*)&expected_checksum)!=1) {
                cerr<<v[2]<<" is not a valid checksum value."<<endl;
                usage();
            }
            
            reader.verifyChecksum(expected_checksum);
        } else {
            reader.verifyChecksum();
        }
    } catch (const exception &e) {
        cerr<<"Error: "<<e.what()<<endl;
        return 1;
    }
    return 0;
}

// **************************************************************************
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 
// 2011, 2012 Purdue University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//  this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are 
// those of the authors and should not be interpreted as representing official
// policies, either expressed or implied, of Purdue University or the VERITAS 
// Collaboration.
// **************************************************************************
