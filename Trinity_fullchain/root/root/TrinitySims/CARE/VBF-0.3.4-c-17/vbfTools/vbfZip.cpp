/*
 * vbfZip.cpp -- compresses a VBF file by applying sample compression
 * by Filip Pizlo, 2005
 */

#include "VBF/VBankFileReader.h"
#include "VBF/VBankFileWriter.h"

#include <iostream>
#include <stdlib.h>

using namespace std;

static void usage() {
    cerr<<"Usage: vbfZip comp|decomp <in filename> <out filename>"<<endl;
    cerr<<"Description:"<<endl;
    cerr<<"Compresses (or decompresses) a VBF file by applying sample compression."<<endl;
    exit(1);
}

int main(int c,char **v) {
    try {
	if (c!=4) {
	    usage();
	}
	
	bool compBit=false;
	
	if (!strcasecmp(v[1],"comp")) {
	    compBit=true;
	} else if (!strcasecmp(v[1],"decomp")) {
	    compBit=false;
	} else {
	    usage();
	}
		
	VBankFileReader reader(v[2],false,true);
	VBankFileWriter writer(v[3],
			       reader.getRunNumber(),
			       reader.getConfigMask());
		
	for (unsigned i=0;
	     reader.hasPacket(i);
	     ++i) {
	    VPacket *packet=reader.readPacket(i);
	    if (packet->has(VGetArrayEventBankName())) {
		VArrayEvent *ae=packet->get< VArrayEvent >(VGetArrayEventBankName());
		for (unsigned i=0;i<ae->getNumEvents();++i) {
		    ae->getEventAt(i)->setCompressedBit(compBit);
		}
	    }
	    if (packet->has(VGetEventOverflowBankName())) {
		VEventOverflow *eo=packet->get< VEventOverflow >(VGetEventOverflowBankName());
		for (unsigned i=0;i<eo->numDatums();++i) {
		    VEvent *ev=dynamic_cast< VEvent* >(eo->getDatumAt(i));
		    if (ev!=NULL) {
			ev->setCompressedBit(compBit);
		    }
		}
	    }
	    writer.writePacket(packet);
	    delete packet;
	}
	
	writer.finish();
		
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
