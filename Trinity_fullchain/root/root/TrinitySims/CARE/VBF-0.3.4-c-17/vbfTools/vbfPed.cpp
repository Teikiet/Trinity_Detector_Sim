/*
 * vbfPed.cpp -- performs a simple pedestal analysis
 * by Filip Pizlo, 2005
 */

#include "VBF/VBankFileReader.h"

#include <math.h>
#include <iostream>
#include <map>
#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

using namespace std;

static void usage() {
    cerr<<"Usage: vbfPed [-t <code> [-a] [-g]] [-d <where>] [-v] <start sample>"<<endl;
    cerr<<"            <end sample> <upper bound> <file1> [<file2> ...]"<<endl;
    cerr<<"Description:"<<endl;
    cerr<<"Performs a pedestal analysis, using the given sample range.  If any of the"<<endl;
    cerr<<"samples in the range are above the given upper bound, that trace is ignored"<<endl;
    cerr<<"for the purposes of the pedestal analysis.  The results of the analysis are"<<endl;
    cerr<<"printed to the console in a four-column format consisting of telescope id,"<<endl;
    cerr<<"channel number, pedestal value, and pedestal standard deviation.  Note that"<<endl;
    cerr<<"the telescope id and channel number are zero-based.  If the -d flag is"<<endl;
    cerr<<"specified, the distribution of pedestal values will be dumped to a file.  If"<<endl;
    cerr<<"the -t flag is specified, only events containing the given type code are"<<endl;
    cerr<<"considered.  If the -a flag is given in addition to the -t flag, L3's event"<<endl;
    cerr<<"type will be used instead of the telescopes' event type.  If the -g flag is"<<endl;
    cerr<<"given, the GPS year field will be used as if it were the event type.  The -v"<<endl;
    cerr<<"flag causes vbfPed to be more verbose."<<endl;
    exit(1);
}

int main(int c,char **v) {
    try {
	const char *dumpTo=NULL;
	bool useL3=false;
	bool useType=false;
	bool useGPSYear=false;
	bool verbose=false;
	unsigned type=0;

	int optCh;
	while ((optCh=getopt(c,v,"d: t: a g v"))!=-1) {
	    switch (optCh) {
	    case 'd':
		dumpTo=strdup(optarg);
		break;
	    case 't':
		if (sscanf(optarg,"%u",&type)!=1) {
		    cerr<<optarg<<" is not a valid argument for -t"<<endl;
		    return 1;
		}
		useType=true;
		break;
	    case 'a':
		useL3=true;
		break;
	    case 'g':
		useGPSYear=true;
		break;
	    case 'v':
		verbose=true;
		break;
	    default:
		usage();
	    }
	}

	if (optind+4>c) {
	    usage();
	}

	unsigned start,end,upper;

	if (sscanf(v[optind],"%u",&start)!=1) {
	    cerr<<v[optind]<<" is not a valid value for <start sample>"<<endl;
	    usage();
	}

	if (sscanf(v[optind+1],"%u",&end)!=1 || end<=start) {
	    cerr<<v[optind+1]<<" is not a valid value for <end sample>"<<endl;
	    usage();
	}

	if (sscanf(v[optind+2],"%u",&upper)!=1) {
	    cerr<<v[optind+2]<<" is not a valid value for <upper bound>"<<endl;
	    usage();
	}

	map< unsigned, map< unsigned, vector< unsigned > > > spectra;

	for (int argi=optind+3;argi<c;++argi) {
	    VBankFileReader reader(v[argi],false,true);
	    
	    for (unsigned i=0;
		 reader.hasPacket(i);
		 ++i) {
		VPacket *packet=reader.readPacket(i);
		if (packet->hasArrayEvent()) {
		    VArrayEvent *ae=packet->getArrayEvent();
		    if (!useType || !useL3 ||
			(ae->hasTrigger() &&
			 ((!useGPSYear && ae->getTrigger()->getEventTypeCode()==type) ||
			  (useGPSYear && ae->getTrigger()->getGPSYear()==type)))) {
			for (unsigned j=ae->getNumEvents();
			     j-->0;) {
			    VEvent *ev=ae->getEventAt(j);
			    if (useType && !useL3 &&
				((!useGPSYear && ev->getEventTypeCode()!=type) ||
				 (useGPSYear && ev->getGPSYear()!=type))) {
				continue;
			    }
			    unsigned my_start=start,my_end=end;
			    if (my_end>ev->getNumSamples()) {
				my_end=ev->getNumSamples();
			    }
			    if (my_start>=my_end) {
				continue;
			    }
			    if (verbose) {
				cerr<<"Processing Event #"<<i
				    <<", Telescope #"<<j<<endl;
			    }
			    for (unsigned k=0,l=0;
				 k<ev->getMaxNumChannels();
				 ++k) {
				if (ev->getHitBit(k)) {
				    if (!ev->getHiLo(l)) {
					const uint8_t *samples=ev->getSamplePtr(l,0);
					bool ok=true;
					for (unsigned m=my_start;m<my_end;++m) {
					    if (samples[m]>upper) {
						ok=false;
						break;
					    }
					}
					if (ok) {
					    vector< unsigned > *spectrum=
						&(spectra[ev->getNodeNumber()][k]);
					    while (spectrum->size()<256) {
						spectrum->push_back(0);
					    }
					    for (unsigned m=my_start;m<my_end;++m) {
						(*spectrum)[samples[m]]++;
					    }
					}
				    }
				    ++l;
				}
			    }
			}
		    }
		}
		delete packet;
	    }
	}

	if (dumpTo!=NULL) {
	    mkdir(dumpTo,0755);
	    for (map< unsigned, map< unsigned, vector< unsigned > > >::iterator
		     i=spectra.begin();
		 i!=spectra.end();
		 ++i) {
		for (map< unsigned, vector< unsigned > >::iterator
			 j=i->second.begin();
		     j!=i->second.end();
		     ++j) {
		    ostringstream buf;
		    buf<<dumpTo<<"/"<<i->first<<"_"<<j->first;
		    FILE *fout=fopen(buf.str().c_str(),"w");
		    if (fout==NULL) {
			cerr<<"Error opening "<<buf.str()<<": "
			    <<strerror(errno)<<endl;
			return 1;
		    }
		    for (unsigned k=0;
			 k<j->second.size();
			 ++k) {
			fprintf(fout,"%u %u\n",k,j->second[k]);
		    }
		    fclose(fout);
		}
	    }
	}
	
	for (map< unsigned, map< unsigned, vector< unsigned > > >::iterator
		 i=spectra.begin();
	     i!=spectra.end();
	     ++i) {
	    for (map< unsigned, vector< unsigned > >::iterator
		     j=i->second.begin();
		 j!=i->second.end();
		 ++j) {
		double sum=0.0,n=0.0;
		for (unsigned k=0;
		     k<j->second.size();
		     ++k) {
		    sum+=k*j->second[k];
		    n+=j->second[k];
		}
		double mean=sum/n;
		double var=0;
		for (unsigned k=0;
		     k<j->second.size();
		     ++k) {
		    var+=j->second[k]*(mean-k)*(mean-k);
		}
		double stddev=sqrt(var/(n-1));
		cout<<i->first<<" "<<j->first<<" "<<mean<<" "<<stddev<<endl;
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
