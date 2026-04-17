/*
 * VBankBuilder.h -- builds banks
 * by Filip Pizlo, 2004, 2005
 */

#ifndef V_BANK_BUILDER_H
#define V_BANK_BUILDER_H

#include "Words.h"
#include "VException.h"

#include "VBankName.h"

class VBankBuilderException: public VException {};

class VBankBuilderBadFormatException: public VBankBuilderException {
 public:
    VBankBuilderBadFormatException(const std::string &msg="") {
	setStrings("Bank builder encountered a format error",msg);
    }
};

class VBankBuilderVersionException: public VBankBuilderException {
 public:
    VBankBuilderVersionException(const std::string &msg="") {
	setStrings("Bank builder encountered an unknown version",msg);
    }
};

class VBankBuilder {
 private:
    VBankBuilder(const VBankBuilder& other) {}
    void operator=(const VBankBuilder& other) {}
    
 public:
    VBankBuilder() {}
    virtual ~VBankBuilder() {}
        
    virtual bool canRead(const VBankName &name) {
	return true;
    }

    // if you encounter a formatting problem, it is preferrable that
    // you throw a VBankBuilderBadFormatException
        
    virtual VBank *readBankFromBuffer(long run_number,
				      uword32 event_number,
				      const VBankName &name,
				      uword32 version,
				      uword32 size,
				      const char *buf) =0;
        
    virtual VBank *readBankFromFile(long run_number,
				    uword32 event_number,
				    const VBankName &name,
				    uword32 version,
				    uword32 size,
				    word64 offset,
				    int fd) =0;
        
};

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
