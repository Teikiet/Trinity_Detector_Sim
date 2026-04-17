/*
 * VBFUtil.h -- utility stuff for VBF
 * by Filip Pizlo, 2004
 */

#ifndef VBF_UTIL_H
#define VBF_UTIL_H

#include "Words.h"

namespace VBFUtil {
    
    // NOTE: the code below may be 'sub-optimal' on little-endian hardware.
    // however, the code below has the nice property that it is alignment-safe.
    // no matter how badly misaligned the word you're writing or reading is,
    // and no matter how strict the alignment requirements of your hardware,
    // this code will still work.  that's why I wrote it the way I wrote it.
    
    inline void wordToBuf(uword32 value,
                          char *buf) {
        buf[0]=value&0xff;
        buf[1]=(value>>8)&0xff;
        buf[2]=(value>>16)&0xff;
        buf[3]=(value>>24)&0xff;
    }
    
    inline void wordToBuf(uword64 value,
                          char *buf) {
        buf[0]=value&0xff;
        buf[1]=(value>>8)&0xff;
        buf[2]=(value>>16)&0xff;
        buf[3]=(value>>24)&0xff;
        buf[4]=(value>>32)&0xff;
        buf[5]=(value>>40)&0xff;
        buf[6]=(value>>48)&0xff;
        buf[7]=(value>>56)&0xff;
    }
    
    inline uword32 bufToWord32(const char *_buf) {
        const unsigned char *buf=(const unsigned char*)_buf;
        return ((uword32)buf[0])
              |(((uword32)buf[1])<<8)
              |(((uword32)buf[2])<<16)
              |(((uword32)buf[3])<<24);
    }
    
    inline uword64 bufToWord64(const char *_buf) {
        const unsigned char *buf=(const unsigned char*)_buf;
        return ((uword64)buf[0])
              |(((uword64)buf[1])<<8)
              |(((uword64)buf[2])<<16)
              |(((uword64)buf[3])<<24)
              |(((uword64)buf[4])<<32)
              |(((uword64)buf[5])<<40)
              |(((uword64)buf[6])<<48)
              |(((uword64)buf[7])<<56);
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
