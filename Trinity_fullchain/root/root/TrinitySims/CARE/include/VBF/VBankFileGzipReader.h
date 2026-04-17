/*
 * VBankFileGzipReader.h -- reads gzip files
 * by Filip Pizlo, 2005
 */

#ifndef V_BANK_FILE_GZIP_READER_H
#define V_BANK_FILE_GZIP_READER_H

#include "VBankFileStreamReader.h"

class VBankFileGzipException: public VBankFileReaderException {
 public:
    VBankFileGzipException(const std::string &msg="") {
	setStrings("Bank file reader experienced a Gzip error",msg);
    }
};

#ifdef HAVE_GZIP
#include <zlib.h>
typedef gzFile V_gzFile;
#else
typedef void *V_gzFile;
#endif

class VBankFileGzipReader: public VBankFileStreamReader {
 private:
    
    int fd;
    bool s_live;
    V_gzFile fl;
    bool eof;
    
    void makeStream();
    void killStream();

 protected:
    
    virtual void readDataImpl(void *buf,size_t len);
    
 public:
    
    VBankFileGzipReader(VBankFileReader *reader,
			int fd);
    
    virtual ~VBankFileGzipReader();
    
    virtual void reset();
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
