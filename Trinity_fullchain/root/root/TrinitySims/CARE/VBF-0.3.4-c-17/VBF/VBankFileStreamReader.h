/*
 * VBankFileStreamReader.h -- reads a bank file from a stream
 * by Filip Pizlo, 2005
 */

#ifndef V_BANK_FILE_STREAM_READER_H
#define V_BANK_FILE_STREAM_READER_H

#include "VBankFileReader.h"

class VBankFileStreamReader: public VBankFileReaderBase {
 protected:
	
	class EndOfFile {};

	// throws EndOfFile on end-of-file.
	virtual void readDataImpl(void *buf,size_t len) =0;

 private:
	
	VBankFileReader *reader;

	uword32 event_number;

	bool has_data_left;
	uword64 data_left;

	void readData(void *buf,size_t len);

	uint8_t readByte();
	uword32 readWord32();
	uword64 readWord64();

	void skip(size_t len);

 public:
	
	VBankFileStreamReader(VBankFileReader *reader);

	virtual ~VBankFileStreamReader();

	void initialize();

	virtual void reset() =0;

	VPacket *readPacket();

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
