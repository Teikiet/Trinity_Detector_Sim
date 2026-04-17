/*
 * vbfSummary.cpp -- prints summary info about a VBF file
 * by Filip Pizlo, 2004
 */

#include "VBF/VBankFileReader.h"
#include "VBF/VConfigMaskUtil.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

static void usage() {
    cerr<<"Usage: vbfSummary <filename>"<<endl;
    cerr<<"Description:"<<endl;
    cerr<<"Prints out a quick summary of the given file.  vbfSummary is designed to"<<endl;
    cerr<<"run in constant time, so it only prints information that is available in"<<endl;
    cerr<<"the header and footer."<<endl;
    exit(1);
}

int main(int c,char **v) {
    try {
        if (c!=2) {
            usage();
        }
        
        VBankFileReader reader(v[1],false,true);
        
        cerr<<"Run Number:         "<<reader.getRunNumber()<<endl;
        cerr<<"Configuration:      "<<
            VConfigMaskUtil::configMaskToString(
                reader.getConfigMask())<<endl;
	cerr<<"Stream Mode:        "<<(reader.isStreamed()?"Yes":"No")<<endl;
        
	if (!reader.isStreamed()) {
	    cerr<<"File Size:          "<<reader.getFileSize()<<" bytes"<<endl;
	    cerr<<"Header Size:        "<<reader.getHeaderSize()<<" bytes"<<endl;
	    cerr<<"Original Body Size: "<<reader.getOriginalBodySize()
		<<" bytes"<<endl;
	    cerr<<"Body Size:          "<<reader.getBodySize()<<" bytes"<<endl;
	    cerr<<"Footer Size:        "<<reader.getFooterSize()<<" bytes"<<endl;
	} else {
	    cerr<<"Header Size:        "<<reader.getHeaderSize()<<" bytes"<<endl;
	}
        
	if (!reader.isStreamed()) {
	    if (reader.hasChecksum()) {
		cerr<<"Has Checksum:       Yes"<<endl;
		cerr<<"Checksum Value:     "<<reader.getChecksum()
		    <<" (use vbfCheck to verify)"<<endl;
	    } else {
		cerr<<"Has Checksum:       No"<<endl;
	    }
	    
	    try {
		if (reader.mapIndex()) {
		    cerr<<"Has Index:          Yes"<<endl;
		    cerr<<"Num Packets:        "<<reader.numPackets()<<endl;
		} else {
		    cerr<<"Has Index:          No"<<endl;
		}
	    } catch (const exception &e) {
		cerr<<"Has Index:          No (got error: "<<e.what()<<")"<<endl;
	    }
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
