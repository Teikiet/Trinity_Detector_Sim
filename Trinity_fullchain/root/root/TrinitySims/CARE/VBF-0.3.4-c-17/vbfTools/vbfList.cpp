/*
 * vbfList.cpp -- lists off the banks that are in all of the packets
 * by Filip Pizlo, 2004, 2005
 */

#include "VBF/VBankFileReader.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

static void usage() {
    cerr<<"Usage: vbf_list <filename>"<<endl;
    cerr<<"Description:"<<endl;
    cerr<<"Prints out a summary of the contents of each packet in the given file.  Only"<<endl;
    cerr<<"the Bank IDs and version numbers are printed."<<endl;
    exit(1);
}

class VersionBank: public VBank {
 private:
    uword32 version;
 public:
    VersionBank(uword32 version):
	version(version)
    {}
    uword32 getBankVersion() {
	return version;
    }
    uword32 getBankSize() {
	V_FAIL("unimplemented");
    }
    void writeBankToBuffer(char *buf) {
	V_FAIL("unimplemented");
    }
    uword32 calcBankAdler(uword32 adler) {
	V_FAIL("unimplemented");
    }
    void writeBankToFile(word64 offset,int fd) {
	V_FAIL("unimplemented");
    }
};

class VersionBuilder: public VBankBuilder {
 public:
    VBank *readBankFromBuffer(long run_number,
			      uword32 event_number,
			      const VBankName &name,
			      uword32 version,
			      uword32 size,
			      const char *buf) {
	return new VersionBank(version);
    }
    VBank *readBankFromFile(long run_number,
			    uword32 event_number,
			    const VBankName &name,
			    uword32 version,
			    uword32 size,
			    off_t offset,
			    int fd) {
	return new VersionBank(version);
    }
};

int main(int c,char **v) {
    try {
	if (c!=2) {
	    usage();
	}
		
	VBankFileReader reader(v[1],false,true);
		
	reader.resetBankBuilders();
	reader.addFinalBankBuilder(new VersionBuilder());
		
	for (unsigned i=0;
	     reader.hasPacket(i);
	     ++i) {
	    VPacket *packet=reader.readPacket(i);

	    cout<<i<<":";

	    for (VPacket::iterator
		     j=packet->begin();
		 j!=packet->end();
		 ++j) {
		VersionBank *vb=dynamic_cast< VersionBank* >(j->second);
		V_ASSERT(vb!=NULL);
		cout<<" "<<j->first.getNameAsString()
		    <<"/"<<vb->getBankVersion();
	    }
			
	    cout<<endl;

	    delete packet;
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
