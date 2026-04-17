/*
 * FPFDWrite.h -- fpio output base that just writes the data out to the
 *                file descriptor.
 * by Filip Pizlo, 2003, 2004
 */

#ifndef FP_FD_WRITE_H
#define FP_FD_WRITE_H

#include "FPIO.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

class FPFDWrite:
    public FPOutputBase,
    public FPEndiannessAware
{
    private:
        
        int fd;
        word64 offset;
    
    public:
        
        FPFDWrite(int fd):
            fd(fd),
            offset(-1)
        {
        }
        
        FPFDWrite(int fd,
                  word64 offset):
            fd(fd),
            offset(offset)
        {
        }
        
        virtual ~FPFDWrite() {
        }
        
        int getOutputEndianness() { return getEndianness(); }
        
        int putBytes(const char *data,int len)  ;
        
        void writeBytes(const char *data,int len)  {
            while (len>0) {
                int cnt=putBytes(data,len);
                data+=cnt;
                len-=cnt;
            }
        }
        
        void writeWordImpl(const char *data,int len)  {
            if (getEndianness() == ENDIANNESS_CONVERT && len!=1) {
                for (int n=len;n-->0;) {
                    writeWord((int8_t)data[n]);
                }
                return;
            }
            
            writeBytes(data,len);
        }
        
        void writeString(const char *str)  {
            writeBytes(str,strlen(str)+1);
        }
        
        void writePadding(int len)  {
            while (len-->0) {
                writeWord((int8_t)0);
            }
        }
        
        int getAvailableForWriting() {
            return 0;
        }
        
        int getBytesWritten() {
            return 0;
        }
        
        int getBytesToBeSent() {
            return 0;
        }
        
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
