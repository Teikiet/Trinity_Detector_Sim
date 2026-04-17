/*
 * FPAdler.h -- adler calculation via FPOutputBase
 * by Filip Pizlo, 2004
 */

#ifndef FPADLER_H
#define FPADLER_H

#include "FPIO.h"
#include "Adler32.h"

class FPAdlerOut:
    public FPOutputBase,
    public FPEndiannessAware
{
    private:
        
        uword32 adler;
        uword32 count;
        
    public:
        
        FPAdlerOut() {
            adler=vbf_adler32(0,NULL,0);
        }
        
        FPAdlerOut(uword32 adler):
            adler(adler)
        {
        }
        
        virtual ~FPAdlerOut() {}
        
        uword32 getAdler() const noexcept {
            return adler;
        }
        
        void writeBytes(const char *data,int len)  {
            adler=vbf_adler32(adler,(const unsigned char*)data,len);
            count+=len;
        }
        
        void writeWordImpl(const char *data,int len)  {
            if (getEndianness()==ENDIANNESS_DONT_CONVERT) {
                writeBytes(data,len);
                return;
            }
            
            for (int i=len;i-->0;) {
                adler=vbf_adler32(adler,(const unsigned char*)data+i,1);
            }
            
            count+=len;
        }
        
        void writeString(const char *str)  {
            writeBytes(str,strlen(str));
        }
        
        void writePadding(int len)  {
            count+=len;
        }
        
        char *requestOutputSpan(int len) {
            return NULL;
        }
        
        int putBytes(const char *data,int len)  {
            writeBytes(data,len);
            return len;
        }
        
        int getAvailableForWriting() {
            return 0;
        }
        
        int getBytesWritten() {
            return count;
        }
        
        int getBytesToBeSent() {
            return count;
        }
        
        int getOutputEndianness() {
            return getEndianness();
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
