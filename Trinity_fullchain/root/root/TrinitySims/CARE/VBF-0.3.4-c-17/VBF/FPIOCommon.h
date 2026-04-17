/*
 * FPIOCommon.h -- stuff that is common to C++ and C FPIO
 * by Filip Pizlo, 2002
 */

#ifndef FPIO_COMMON_H
#define FPIO_COMMON_H

#include "Words.h"

#ifdef __cplusplus
extern "C" {
#endif

// functions from Stevens
int set_fl(int fd,int flags);
int clr_fl(int fd,int flags);

#ifdef __cplusplus
};
#endif

// my ultra-cheap shortcuts
#define FP_makeNonBlocking(fd) (set_fl(fd,O_NONBLOCK))
#define FP_makeBlocking(fd) (clr_fl(fd,O_NONBLOCK))

// some new helpers

#define attemptAndRetry(a,b) ((a)<0?((b),(a)):0)
#define assertWrite(out,command) ((out->command)<0?(out->resetPacket(),return -1):0)

// totally magic stuff

#define FP_min(a,b) ((a)<(b)?(a):(b))
#define FP_max(a,b) ((a)>(b)?(a):(b))
#define FP_limit(x,i,a) ((x)<(i)?(i):((x)>(a)?(a):(x)))

#define FPQueue_rollingDiff(put,get,size) \
 ((put)>(get)?(put)-(get):(size)-(get)+(put))

#define DEFAULT_BUFFER_SIZE 65536
#define ENDIANNESS_DONT_CONVERT 0
#define ENDIANNESS_CONVERT 1
#define ENDIANNESS_BIG_ENDIAN 2
#define ENDIANNESS_LITTLE_ENDIAN 3

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
