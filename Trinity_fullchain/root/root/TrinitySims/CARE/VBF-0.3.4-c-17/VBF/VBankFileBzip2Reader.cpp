/*
 * VBankFileBzip2Reader.cpp -- impl
 * by Filip Pizlo, 2005
 */

#include "VBankFileBzip2Reader.h"

#include <iostream>
#include <unistd.h>

using namespace std;

#ifdef HAVE_BZIP2

#define BUF_SIZE 65536

static const char *libbzip2_strerror(int code) {
    switch (code) {
    case BZ_CONFIG_ERROR:   return "libbzip2 is not configured to run on"
	                           "this platform.  This is a fatal error.";
    case BZ_SEQUENCE_ERROR: return "Incorrect sequence of calls to "
	                           "libbzip2 functions.  This is a fatal "
	                           "error.";
    case BZ_PARAM_ERROR:    return "Parameter to libbzip2 function is out "
	                           "of range.  This is a fatal error.";
    case BZ_MEM_ERROR:      return "Attempt to request memory failed.";
    case BZ_DATA_ERROR:     return "Data integrity error.";
    case BZ_DATA_ERROR_MAGIC: return "Data does not contain correct magic "
	                           "bytes.";
    case BZ_IO_ERROR:       return "I/O error.  Check system's errno.";
    case BZ_UNEXPECTED_EOF: return "Unexpected end of file.";
    case BZ_OUTBUFF_FULL:   return "Output buffer is too small.";
    default:                return "Unknown error code.";
    }
}

void VBankFileBzip2Reader::makeStream() {
    if (lseek(fd,0l,SEEK_SET)<0) {
	throw VSystemException
	    ("Unable to seek in VBankFileBzip2Reader::makeStream()");
    }
	
    s->bzalloc=NULL;
    s->bzfree=NULL;
    s->opaque=NULL;
    s->avail_in=0;
	
    int res=BZ2_bzDecompressInit(s,0,0);
    if (res!=BZ_OK) {
	throw VBankFileBzip2Exception(libbzip2_strerror(res));
    }
	
    s_live=true;
    eof=false;
}

void VBankFileBzip2Reader::killStream() {
    if (!s_live) {
	return;
    }
	
    BZ2_bzDecompressEnd(s);
	
    s_live=false;
}

void VBankFileBzip2Reader::readDataImpl(void *buf,
					size_t len) {
    if (!s_live) {
	throw VBankFileBzip2Exception
	    ("Cannot do readDataImpl() because of previous errors");
    }

    if (len==0) {
	return;
    }
	
    if (eof) {
	//cerr<<"early eof!"<<endl;
	throw EndOfFile();
    }

    //cerr<<"proceeding with read..."<<endl;
	
    s->next_out=(char*)buf;
    s->avail_out=len;
	
    for (;;) {
	//cerr<<"in loop."<<endl;
		
	if (s->avail_in!=0) {
	    //cerr<<"calling decomp"<<endl;
	    int res=BZ2_bzDecompress(s);
	    switch (res) {
	    case BZ_OK:
	    case BZ_RUN_OK:
	    case BZ_FLUSH_OK:
	    case BZ_FINISH_OK:
		//cerr<<"ok!"<<endl;
		break;
	    case BZ_STREAM_END:
		//cerr<<"end!"<<endl;
		eof=true;
		if (s->avail_out==0) {
		    return;
		} else {
		    throw EndOfFile();
		}
	    default:
		throw VBankFileBzip2Exception(libbzip2_strerror(res));
	    }
			
	    if (s->avail_out==0) {
		return;
	    }
	}
		
	//cerr<<"reading..."<<endl;
		
	int res=read(fd,my_buf,my_len);
	if (res==-1) {
	    throw VSystemException("In VBankFileBzip2Reader::readDataImpl()");
	}

	if (res==0) {
	    //cerr<<"eof!"<<endl;
	    throw EndOfFile();
	}
		
	s->next_in=my_buf;
	s->avail_in=res;
    }
}

VBankFileBzip2Reader::VBankFileBzip2Reader(VBankFileReader *reader,
					   int fd):
    VBankFileStreamReader(reader),
    fd(fd),
    s_live(false),
    s(new V_bz_stream()),
    my_buf((char*)malloc(BUF_SIZE)),
    my_len(BUF_SIZE)
{
    try {
	makeStream();
    } catch (...) {
	free(my_buf);
    }
}

VBankFileBzip2Reader::~VBankFileBzip2Reader() {
    killStream();
    free(my_buf);
    delete s;
}

void VBankFileBzip2Reader::reset() {
    killStream();
    makeStream();
}

#else

void VBankFileBzip2Reader::makeStream() {
    V_FAIL("shouldn't be here");
}

void VBankFileBzip2Reader::killStream() {
    V_FAIL("shouldn't be here");
}

void VBankFileBzip2Reader::readDataImpl(void *buf,size_t len) {
    V_FAIL("shouldn't be here");
}

VBankFileBzip2Reader::VBankFileBzip2Reader(VBankFileReader *reader,
					   int fd):
    VBankFileStreamReader(reader)
{
    throw VBankFileBzip2Exception("libbzip2 support not available");
}

VBankFileBzip2Reader::~VBankFileBzip2Reader() {}

void VBankFileBzip2Reader::reset() {
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
