/*
 * VEventOverflow.h -- stores excess events (and triggers)
 * by Filip Pizlo, 2005
 *
 * Note that this is only used for VBF files.
 */

#ifndef V_EVENT_OVERFLOW_H
#define V_EVENT_OVERFLOW_H

#include "VDatum.h"
#include "VArrayEvent.h"
#include "VBank.h"
#include "VBankBuilder.h"

#include <vector>

class VEventOverflow: public VBank {
 private:
    std::vector< VDatum* > data;
		
 public:
    VEventOverflow() {}
    virtual ~VEventOverflow();
		
    void addDatum(VDatum *datum) {
	data.push_back(datum);
    }
		
    void addArrayEvent(VArrayEvent *ae) {
	for (unsigned i=0;i<ae->getNumEvents();++i) {
	    addDatum(ae->getEventAt(i));
	}
	if (ae->hasTrigger()) {
	    addDatum(ae->getTrigger());
	}
    }
		
    unsigned numDatums() const noexcept {
	return data.size();
    }
		
    VDatum *getDatumAt(unsigned i) const noexcept {
	return data[i];
    }

    void replaceDatumAt(unsigned i,
			VDatum *dt) {
	delete data[i];
	data[i]=dt;
    }
		
    // bank file functions
    uword32 getBankVersion();
    void verifyBank(long run_number,
		    const std::vector< bool > &config_mask);
    uword32 getBankSize();
    void writeBankToBuffer(char *buf);
    uword32 calcBankAdler(uword32 adler);
    void writeBankToFile(word64 offset,
			 int fd);

};

const VBankName &VGetEventOverflowBankName();
VBankBuilder *VGetEventOverflowBankBuilder();

#endif


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
