/*
 * FPFDRead.h -- fpio input base that just sucks stuff directly out of a
 *               file descriptor
 * by Filip Pizlo, 2003, 2004
 */

#ifndef FP_FD_READ_H
#define FP_FD_READ_H

#include "FPIO.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

class FPFDRead:
    public FPInputBase,
    public FPEndiannessAware
{
    private:
        
        int fd;
        word64 offset;
        uword32 count;
        
    public:
        
        FPFDRead(int _fd):
            fd(_fd),
            offset(-1),
            count(0)
        {
        }
        
        FPFDRead(int _fd,
                 word64 offset):
            fd(_fd),
            offset(offset),
            count(0)
        {
        }
        
        virtual ~FPFDRead() {
        }
        
        int getInputEndianness() { return getEndianness(); }
        
        int readInto(FPTargettable *target)  {
            throw FPNotImplementedException("In FPFDRead::readInto()");
        }
        
        void readBytes(char *data,int len)  ;
        
        void readString(char *data,int len)  {
            int i=0;
            len--;
            
            data[len]=0;
            
            for (;;) {
                int8_t c=readByte();
                
                if (i<=len) {
                    data[i]=c;
                    i++;
                } else {
                    throw FPTargetFullException("In FPInput::readString()");
                }
            }
        }
        
        void readWordImpl(char *data,int len)  {
            readBytes(data,len);
            
            if (getEndianness()==ENDIANNESS_CONVERT) {
                for (int i=0;i<len/2;i++) {
                    std::swap(data[i],data[len-1-i]);
                }
            }
        }
        
        int getBytesAvailable() {
            return 0;
        }
        
        int getBytesReceived() {
            return count;
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
