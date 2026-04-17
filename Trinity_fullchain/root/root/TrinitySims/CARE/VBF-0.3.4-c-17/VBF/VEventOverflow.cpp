/*
 * VEventOverflow.cpp -- stores excess events
 * by Filip Pizlo, 2005
 */

#include "VEventOverflow.h"
#include "FPBasics.h"
#include "FPFDRead.h"
#include "FPFDWrite.h"
#include "FPAdler.h"
#include <unistd.h>
#include <sys/types.h>

using namespace std;

static VEventOverflow *buildBankFrom(uword32 version,
				     uword32 size,
				     FPInputBase *in) {
    VDatum::Version versionEnum;
    switch (version) {
    case 0:
	versionEnum=VDatum::AUG_2004;
	break;
    case 1:
	versionEnum=VDatum::AUG_2005;
	break;
    default:
	throw VBankBuilderVersionException("While building VEventOverflow");
    }
    VEventOverflow *result=new VEventOverflow();
    while (in->getBytesReceived()<(int)size) {
	result->addDatum(VDatumParser::buildFrom(in,versionEnum));
    }
    // FIXME: maybe check the event numbers?
    return result;
}

class VEventOverflowBankBuilder:
    public VBankBuilder
{
 public:
		
    VBank *readBankFromBuffer(long run_number,
			      uword32 event_number,
			      const VBankName &name,
			      uword32 version,
			      uword32 size,
			      const char *buf) {
	FPBasicIn in(buf,size);
	return buildBankFrom(version,size,&in);
    }
		
    VBank *readBankFromFile(long run_number,
			    uword32 event_number,
			    const VBankName &name,
			    uword32 version,
			    uword32 size,
			    off_t offset,
			    int fd) {
	FPFDRead in(fd,offset);
	return buildBankFrom(version,size,&in);
    }
		
};

static VBankName bank_name("CoreOvrf");
static VEventOverflowBankBuilder bank_builder;

const VBankName &VGetEventOverflowBankName() {
    return bank_name;
}

VBankBuilder *VGetEventOverflowBankBuilder() {
    return &bank_builder;
}

VEventOverflow::~VEventOverflow() {
    for (unsigned i=data.size();i-->0;) {
	delete data[i];
    }
}

uword32 VEventOverflow::getBankVersion() {
    return 0;
}

void VEventOverflow::verifyBank(long run_number,
				const vector< bool > &config_mask) {
}

uword32 VEventOverflow::getBankSize() {
    uword32 result=0;
    for (unsigned i=0;i<data.size();++i) {
	result+=data[i]->getSize();
    }
    return result;
}

void VEventOverflow::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    for (unsigned i=0;i<data.size();++i) {
	data[i]->writeTo(&out);
    }
}

uword32 VEventOverflow::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    for (unsigned i=0;i<data.size();++i) {
	data[i]->writeTo(&out);
    }
    return out.getAdler();
}

void VEventOverflow::writeBankToFile(word64 offset,
				     int fd) {
    FPFDWrite out(fd,offset);
    for (unsigned i=0;i<data.size();++i) {
	data[i]->writeTo(&out);
    }
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
