/*
 * VBankFileGzipReader.cpp -- impl
 * by Filip Pizlo, 2005
 */

#include "VBankFileGzipReader.h"

#include <iostream>
#include <unistd.h>

using namespace std;

#ifdef HAVE_GZIP

void VBankFileGzipReader::makeStream() {
    if (lseek(fd,0l,SEEK_SET)<0) {
	throw VSystemException
	    ("Unable to seek in VBankFileGzipReader::makeStream()");
    }
    
    fl=gzdopen(dup(fd),"rb");
    if (fl==NULL) {
	throw VBankFileGzipException
	    ("gzdopen() failed to allocate the stream");
    }

    s_live=true;
    eof=false;
}

void VBankFileGzipReader::killStream() {
    if (!s_live) {
	return;
    }
    
    gzclose(fl);
    
    s_live=false;
}

void VBankFileGzipReader::readDataImpl(void *buf,
				       size_t len) {
    if (!s_live) {
	throw VBankFileGzipException
	    ("Cannot do readDataImpl() because of previous errors");
    }
    
    if (len==0) {
	return;
    }
    
    if (eof) {
	throw EndOfFile();
    }

    char *cur=(char*)buf;
    while (len>0) {
	int res=gzread(fl,cur,len);
	if (res<0) {
	    int _;
	    throw VBankFileGzipException(gzerror(fl,&_));
	}
	if (res==0) {
	    eof=true;
	    throw EndOfFile();
	}
	cur+=res;
	len-=res;
    }
}

VBankFileGzipReader::VBankFileGzipReader(VBankFileReader *reader,
					 int fd):
    VBankFileStreamReader(reader),
    fd(fd),
    s_live(false)
{
    makeStream();
}

VBankFileGzipReader::~VBankFileGzipReader() {
    killStream();
}

void VBankFileGzipReader::reset() {
    killStream();
    makeStream();
}

#else

void VBankFileGzipReader::makeStream() {
    V_FAIL("shouldn't be here");
}

void VBankFileGzipReader::killStream() {
    V_FAIL("shouldn't be here");
}

void VBankFileGzipReader::readDataImpl(void *buf,size_t size) {
    V_FAIL("shouldn't be here");
}

VBankFileGzipReader::VBankFileGzipReader(VBankFileReader *reader,
					 int fd):
    VBankFileStreamReader(reader)
{
    throw VBankFileGzipException("zlib support not available");
}

VBankFileGzipReader::~VBankFileGzipReader() {}

void VBankFileGzipReader::reset() {
    V_FAIL("shouldn't be here");
}

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
