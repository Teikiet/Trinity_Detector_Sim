/*
 * FPBasics.h -- base primitives of FPIO functions
 * by Filip Pizlo, 2002
 */

#ifndef FPBASICS_H
#define FPBASICS_H

#include "FPIO.h"

/*
 * Writes a bunch of data to a buffer and optinally places a 32-bit word at the
 * beginning of the buffer, with all apropriate conversion, that specifies
 * the length of data actually written.
 */
class FPBasicOut:
    public FPOutputBase,
    public FPEndiannessAware
{
private:
        
    char *buffer;
    int max_len;
    int cursor;
        
    bool place_count;
        
public:
        
    FPBasicOut();
        
    FPBasicOut(char *_buffer,int _max_len,bool _place_count=true)  ;
        
    virtual ~FPBasicOut();
        
    void setTarget(char *_buffer,int _max_len,bool _place_count=true)  ;
        
    virtual void writeBytes(const char *data,int len)  ;
        
    virtual void writeWordImpl(const char *data,int len)  ;
        
    virtual void writeString(const char *str)  ;
        
    virtual void writePadding(int len)  ;
        
    virtual char *requestOutputSpan(int len);
        
    virtual int putBytes(const char *data,int len)  ;
        
    virtual int getAvailableForWriting() {
	return max_len-cursor;
    }
        
    virtual int getBytesWritten() {
	return place_count?cursor-4:cursor;
    }
        
    virtual int getBytesToBeSent() {
	return cursor;
    }
        
    void finishWriting();
        
    int getOutputEndianness() {
	return getEndianness();
    }

};

/*
 * Takes a buffer and a length and allows user to read it using FPIO
 * methods.  Does not do anything with any 32-bit words that indicate
 * length.
 */
class FPBasicIn:
    public FPInputBase,
    public FPEndiannessAware
{
private:

    const char *orig_begin;
    const char *begin,*end;
        
public:
        
    FPBasicIn();
        
    FPBasicIn(const char *buffer,int len)  ;
        
    virtual ~FPBasicIn();
        
    void setSource(const char *buffer,int len)  ;
        
    virtual int getInputEndianness();
        
    virtual int readInto(FPTargettable *buf)  ;
        
    virtual void readBytes(char *data,int len)  ;
        
    virtual void readString(char *data,int len)  ;
        
    virtual void readWordImpl(char *data,int len)  ;
        
    virtual const char *requestInputSpan(int len);
        
    virtual int getBytesAvailable();
        
    virtual int getBytesReceived();
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
