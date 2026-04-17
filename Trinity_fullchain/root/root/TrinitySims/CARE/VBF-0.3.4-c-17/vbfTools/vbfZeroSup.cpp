/*
 * vbfZeroSup.cpp -- performs zero suppression.
 * by Filip Pizlo, 2005
 */

#include "VBF/VBankFileReader.h"
#include "VBF/VBankFileWriter.h"

#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <map>

using namespace std;

static void usage() {
    cerr<<"Usage: vbfZeroSup [-i] [-t <code>] <thresholds filename> <input filename>"<<endl;
    cerr<<"           <output filename>"<<endl;
    cerr<<"Description:"<<endl;
    cerr<<"Performs software zero-suppression.  The thresholds file is expected to"<<endl;
    cerr<<"contain three integers per line, representing the tuple (telescope id, channel"<<endl;
    cerr<<"number, threshold).  The telescope id and channel number are zero-based.  If"<<endl;
    cerr<<"none of the samples of a particular channel are at or above the threshold and"<<endl;
    cerr<<"the channel's hi/lo bit is not set, the channel is zero-suppressed.  Note that"<<endl;
    cerr<<"if a particular telescope/channel combination does not appear in the thresholds"<<endl;
    cerr<<"file, that channel is assumed to be always zero-suppressed."<<endl;
    exit(1);
}

int main(int c,char **v) {
    try {
	bool keepPedEvents=true;
	unsigned pedType=10;

	int optCh;
	while ((optCh=getopt(c,v,"t: i"))!=-1) {
	    switch (optCh) {
	    case 't':
		if (sscanf(optarg,"%u",&pedType)!=1) {
		    cerr<<optarg<<" is not a valid event type."<<endl;
		    return 1;
		}
		break;
	    case 'i':
		keepPedEvents=false;
		break;
	    default:
		usage();
		break;
	    }
	}

	if (optind+3>c) {
	    usage();
	}

	map< unsigned, map< unsigned, unsigned > > thresholds;

	FILE *flin=fopen(v[optind],"r");
	if (flin==NULL) {
	    cerr<<"Error: "<<strerror(errno)<<endl;
	    return 1;
	}
	for (;;) {
	    char buf[256];
	    if (fgets(buf,256,flin)==NULL) {
		break;
	    }
	    unsigned tele,chan;
	    double threshold;
	    if (sscanf(buf,"%u %u %lf",&tele,&chan,&threshold)!=3) {
		cerr<<"Parse error in "<<v[optind]<<endl;
		return 1;
	    }
	    thresholds[tele][chan]=(unsigned)ceil(threshold);
	}

	VBankFileReader reader(v[optind+1],false,true);
	VBankFileWriter writer(v[optind+2],
			       reader.getRunNumber(),
			       reader.getConfigMask());
	
	// statistics on zero-suppression.
	double sum=0.0,n=0.0;

	for (unsigned i=0;
	     reader.hasPacket(i);
	     ++i) {
	    VPacket *pack=reader.readPacket(i);
	    if (pack->hasArrayEvent()) {
		VArrayEvent *ae=pack->getArrayEvent();
		if (keepPedEvents ||
		    !ae->hasTrigger() ||
		    ae->getTrigger()->getEventTypeCode()!=pedType) {
		    for (unsigned j=0;
			 j<ae->getNumEvents();
			 ++j) {
			VEvent *ev=ae->getEventAt(j);
			
			map< unsigned, map< unsigned, unsigned > >::iterator
			    iter=thresholds.find(ev->getNodeNumber());
			
			unsigned num_kept=0;
			vector< bool > kept(ev->getMaxNumChannels(),false);
			for (unsigned k=0,l=0;k<ev->getMaxNumChannels();++k) {
			    if (ev->getHitBit(k)) {
				map< unsigned, unsigned >::iterator
				    iter2=iter->second.find(k);
				if (iter2!=iter->second.end()) {
				    bool over=false;
				    if (ev->getHiLo(l)) {
					over=true;
				    } else {
					for (int m=ev->getNumSamples();
					     m-->0;) {
					    if (ev->getSample(l,m)
						>=iter2->second) {
						over=true;
						break;
					    }
					}
				    }
				    if (over) {
					kept[k]=true;
					num_kept++;
				    }
				}
				++l;
			    }
			}

			n++;
			sum+=num_kept;
			
			VEvent *newEv=ev->copyEvent();
			newEv->resizeChannelData(newEv->getNumSamples(),
						 num_kept);
			for (unsigned k=0,l=0,m=0;k<ev->getMaxNumChannels();++k) {
			    if (ev->getHitBit(k)) {
				if (kept[k]) {
				    newEv->setCharge(m,ev->getCharge(l));
				    newEv->setPedestalAndHiLo(m,ev->getPedestalAndHiLo(m));
				    memcpy(newEv->getSamplePtr(m,0),
					   ev->getSamplePtr(l,0),
					   ev->getNumSamples());
				    ++m;
				}
				++l;
			    }
			    newEv->setHitBit(k,kept[k]);
			}
			
			//cerr<<"flags = "<<newEv->getFlags()<<", compressed = "<<newEv->getCompressedBit()<<endl;

			ae->replaceEventAt(j,newEv);
		    }
		}
	    }
	    writer.writePacket(pack);
	    delete pack;
	}

	cerr<<"Average number of channels kept: "<<(sum/n)<<endl;

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
