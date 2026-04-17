/*
 * vbfRepTel.cpp -- replicates data for a telescope
 * by Filip Pizlo, 2005
 */

#include "VBF/VBankFileReader.h"
#include "VBF/VBankFileWriter.h"
#include "VBF/VConfigMaskUtil.h"

#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace VConfigMaskUtil;

static void usage() {
    cerr<<"Usage: vbfRepTel <source tel> <target tels> <input flnm> <output flnm>"<<endl;
    cerr<<"Description:"<<endl;
    cerr<<"Creates bogus stereo data by replicating the data for a particular telescope."<<endl;
    exit(1);
}

int main(int c,char **v) {
    try {
	if (c<5) {
	    usage();
	}

	int src_tele;
	
	if (sscanf(v[1],"%u",&src_tele)!=1) {
	    cerr<<"Cannot parse source telescope."<<endl;
	    return 1;
	}
	
	vector< bool > targs;
	try {
	    targs=parseConfigMask(v[2]);
	} catch (const exception &e) {
	    cerr<<"Target telescopes not parsable: "<<e.what()<<endl;
	    return 1;
	}
	
	VBankFileReader reader(v[3],false,true);

	vector< bool > origs=reader.getConfigMask();
	
	if (!origs[src_tele]) {
	    cerr<<"Source telescope not found in the source data."<<endl;
	    return 1;
	}
	
	vector< bool > overlap=intersect(targs,origs);
	
	if (cardinality(overlap)>0) {
	    cerr<<"Some of the target telescopes are already in the data: "
		<<configMaskToString(overlap)<<endl;
	    return 1;
	}

	VBankFileWriter writer(v[4],
			       reader.getRunNumber(),
			       unify(targs,origs));

	for (unsigned i=0;
	     reader.hasPacket(i);
	     ++i) {
	    VPacket *pack=reader.readPacket(i);
	    if (pack->hasArrayEvent()) {
		VArrayEvent *ae=pack->getArrayEvent();
		if (ae->hasTrigger()) {
		    VArrayTrigger *at=ae->getTrigger();
		    at->setTriggerMask(at->getTriggerMask()|toDAQMask(targs));
		    at->setConfigMask(at->getConfigMask()|toDAQMask(targs));
		    at->resizeSubarrayTelescopes(at->getNumSubarrayTelescopes()+cardinality(targs));
		    at->resizeTriggerTelescopes(at->getNumTriggerTelescopes()+cardinality(targs));
		    for (unsigned j=at->getNumSubarrayTelescopes()-cardinality(targs),k=0;
			 j<at->getNumSubarrayTelescopes();
			 ++j,++k) {
			at->setSubarrayTelescopeId(j,ith(targs,k));
		    }
		    for (unsigned j=at->getNumTriggerTelescopes()-cardinality(targs),k=0;
			 j<at->getNumTriggerTelescopes();
			 ++j,++k) {
			at->setTriggerTelescopeId(j,ith(targs,k));
		    }
		}
		VEvent *the_ev=NULL;
		for (unsigned i=ae->getNumEvents();i-->0;) {
		    VEvent *ev=ae->getEventAt(i);
		    if (ev->getTriggerMask()&(1<<src_tele)) {
			ev->setTriggerMask(ev->getTriggerMask()|
					   toDAQMask(targs));
		    }
		    if (ev->getNodeNumber()==src_tele) {
			the_ev=ev;
		    }
		}
		if (the_ev!=NULL) {
		    for (unsigned i=cardinality(targs);
			 i-->0;) {
			VEvent *ev=the_ev->copyEvent();
			ev->setNodeNumber(ith(targs,i));
			V_ASSERT(ev->getEventNumber()==the_ev->getEventNumber());
			ae->addEvent(ev);
		    }
		}
	    }
	    writer.writePacket(pack);
	    delete pack;
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
