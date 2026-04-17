/*
 * VBankFileStreamReader.cpp -- impl!
 * by Filip Pizlo, 2005
 */

#include "VBankFileStreamReader.h"
#include "VBFUtil.h"

#include <iostream>

using namespace std;
using namespace VBFUtil;

void VBankFileStreamReader::readData(void *buf,size_t len) {
    if (has_data_left && len>data_left) {
	throw EndOfFile();
    }
    readDataImpl(buf,len);
    if (has_data_left) {
	data_left-=len;
    }
}

uint8_t VBankFileStreamReader::readByte() {
    uint8_t b;
    readData(&b,1);
    return b;
}

uword32 VBankFileStreamReader::readWord32() {
    char buf[4];
    readData(buf,4);
    return bufToWord32(buf);
}

uword64 VBankFileStreamReader::readWord64() {
    char buf[8];
    readData(buf,8);
    return bufToWord64(buf);
}

void VBankFileStreamReader::skip(size_t len) {
    while (len>1024) {
	skip(1024);
	len-=1024;
    }
    void *buf=alloca(len);
    readData(buf,len);
}

void VBankFileStreamReader::initialize() {
    try {
	has_data_left=false;
		
	// offset = 0

	char magic_buf[4];
	readData(magic_buf,4);
	if (memcmp(magic_buf,"VBFF",4)) {
	    throw VBankFileReaderBadMagicNumberException
		("In VBankFileStreamReader::initialize()");
	}

	// offset = 4
		
	if (readWord32()!=0) {
	    throw VBankFileReaderBadVersionNumberException
		("In VBankFileStreamReader::initialize()");
	}
		
	// offset = 8

	run_number=readWord32();

	// offset = 12
		
	for (unsigned i=0;
	     i<32;
	     ++i) {
	    uint8_t value=readByte();
	    for (unsigned j=0;
		 j<(i<31?8:7);
		 ++j) {
		config_mask.push_back((bool)(value&(1<<j)));
	    }
	}

	// offset = 44;

	skip(4);

	// offset = 48

	data_left=readWord64();
	if (has_data_left=data_left!=0) {
	    data_left-=56;
	}
		
	// offset = 56
		
	event_number=0;
    } catch (const EndOfFile&) {
	throw VBankFileReaderBadFormatException
	    ("In VBankFileStreamReader::initialize(): unexpected end of file");
    }
}

VBankFileStreamReader::VBankFileStreamReader(VBankFileReader *reader):
    reader(reader)
{}

VBankFileStreamReader::~VBankFileStreamReader() {}

VPacket *VBankFileStreamReader::readPacket() {
    try {
	char magic_buf[4];
	
	readData(magic_buf,4);
	if (memcmp(magic_buf,"VPCK",4)) {
	    throw VBankFileReaderBadFormatException
		("In VBankFileStreamReader::readPacket(): "
		 "bad magic header for packet");
	}
	
	word32 size_left=readWord32()-8;

	VPacket *result=new VPacket();
	try {
	    while (size_left>0) {
		char name_buf[8];
		readData(name_buf,8);
			
		uword32 version=readWord32();
		
		VBankName bank_name(name_buf);
		if (result->has(bank_name)) {
		    throw VBankFileReaderBadFormatException
			("In VBankFileStreamReader::readPacket(): "
			 "duplicate bank name");
		}

		uword32 bank_size=readWord32();

		char *buf=(char*)malloc(bank_size-16);
		try {
		    readData(buf,bank_size-16);
			
		    VBankBuilder *builder=reader->getBankBuilder(bank_name);
		    if (builder!=NULL) {
			result->put(bank_name,
				    builder->readBankFromBuffer(run_number,
								event_number,
								bank_name,
								version,
								bank_size-16,
								buf));
		    }
			
		    free(buf);
		} catch (...) {
		    free(buf);
		    throw;
		}

		size_left-=bank_size;
	    }
	
	    event_number++;

	    return result;
	} catch (...) {
	    delete result;
	    throw;
	}
    } catch (const EndOfFile&) {
	return NULL;
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
