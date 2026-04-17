/*
 * FPBasics.cpp -- base primitives for FPIO functions, impl
 * by Filip Pizlo, 2002
 */

#include "FPBasics.h"

FPBasicOut::FPBasicOut():
    buffer(NULL),
    max_len(0),
    cursor(0)
{
}

FPBasicOut::FPBasicOut(char *_buffer,int _max_len,bool _place_count)  {
    setTarget(_buffer,_max_len,_place_count);
}

void FPBasicOut::setTarget(char *_buffer,int _max_len,bool _place_count)  {
    place_count=_place_count;

    if (place_count&&_max_len<4) {
        throw FPInvalidArgumentException(
            "In FPBasicOut::setTarget: Max len must be at least 4 bytes "
            "when place_count == true");
    }
    
    buffer=_buffer;
    max_len=_max_len;
    cursor=place_count?4:0;
}

FPBasicOut::~FPBasicOut() {
}

void FPBasicOut::writeBytes(const char *data,int len)  {
    if (cursor+len>max_len) {
        throw FPBufferFullException("In FPBasicOut::writeBytes");
    }
    
    memcpy(buffer+cursor,data,len);
    
    cursor+=len;
}

void FPBasicOut::writeWordImpl(const char *data,int len)  {
    if (getEndianness()==ENDIANNESS_DONT_CONVERT) {
        writeBytes(data,len);
        return;
    }
    
    if (cursor+len>max_len) {
        throw FPBufferFullException("In FPBasicOut::writeWordImpl");
    }

    for (int i=0;i<len;i++) {
        buffer[cursor+i]=data[len-i-1];
    }
    
    cursor+=len;
}

void FPBasicOut::writeString(const char *str)  {
    writeBytes((char*)str,strlen(str));
}

void FPBasicOut::writePadding(int len)  {
    if (cursor+len>max_len) {
        throw FPBufferFullException("In FPBasicOut::writePadding");
    }
    
    cursor+=len;
}

char *FPBasicOut::requestOutputSpan(int len) {
    if (cursor+len>max_len) {
        return NULL;
    }
    
    char *ret=buffer+cursor;
    
    cursor+=len;
    
    return ret;
}

int FPBasicOut::putBytes(const char *data,int len)  {
    if (getAvailableForWriting()==0) {
        throw FPBufferFullException("In FPBasicOut::putBytes");
    }
    
    if (len>getAvailableForWriting()) {
        len=getAvailableForWriting();
    }
    
    writeBytes(data,len);
    
    return len;
}

void FPBasicOut::finishWriting() {
    if (max_len==0) {
        return;
    }
    
    if (place_count) {
        word32 size=cursor-4;
        
        if (getEndianness()==ENDIANNESS_CONVERT) {
            buffer[0]=((int8_t*)&size)[3];
            buffer[1]=((int8_t*)&size)[2];
            buffer[2]=((int8_t*)&size)[1];
            buffer[3]=((int8_t*)&size)[0];
        } else {
            buffer[0]=((int8_t*)&size)[0];
            buffer[1]=((int8_t*)&size)[1];
            buffer[2]=((int8_t*)&size)[2];
            buffer[3]=((int8_t*)&size)[3];
        }
    }
}

FPBasicIn::FPBasicIn():
    orig_begin(NULL),
    begin(NULL),
    end(NULL)
{
}

FPBasicIn::FPBasicIn(const char *buffer,int len)  :
    orig_begin(buffer),
    begin(buffer),
    end(buffer+len)
{
}

FPBasicIn::~FPBasicIn() {
}

void FPBasicIn::setSource(const char *buffer,int len)  {
    orig_begin=buffer;
    begin=buffer;
    end=buffer+len;
}

int FPBasicIn::getInputEndianness() {
    return getEndianness();
}

int FPBasicIn::readInto(FPTargettable *buf)  {
    int res=buf->putBytes(begin,end-begin);
    begin+=res;
    return res;
}

void FPBasicIn::readBytes(char *data,int len)  {
    if (len>end-begin) {
        throw FPNotEnoughDataException("In FPBasicIn::readBytes()");
    }
    
    memcpy(data,begin,len);
    
    begin+=len;
}

void FPBasicIn::readString(char *data,int len)  {
    if (begin==end) {
        throw FPNotEnoughDataException("In FPBasicIn::readString()");
    }
    
    char *cur_data=data;
    const char *cur_begin=begin;
    while (cur_data<data+len &&
           cur_begin<end)
    {
        *cur_data=*cur_begin;
        if (*cur_data==0) {
            begin=cur_begin;
            return;
        }
        cur_data++;
        cur_begin++;
    }
    
    if (cur_data==data+len) {
        throw FPTargetFullException("In FPBasicIn::readString()");
    } else /* (cur_begin==end) */ {
        throw FPBadFormatException("In FPBasicIn::readString()");
    }
}

void FPBasicIn::readWordImpl(char *data,int len)  {
    if (getEndianness()==ENDIANNESS_DONT_CONVERT) {
        readBytes(data,len);
        return;
    }
    
    if (len>end-begin) {
        throw FPNotEnoughDataException("In FPBasicIn::readBytes()");
    }
    
    for (int i=0;i<len;i++) {
        data[i]=begin[len-i-1];
    }
    
    begin+=len;
}

const char *FPBasicIn::requestInputSpan(int len) {
    if (len>end-begin) {
        return NULL;
    }
    
    const char *ret=begin;
    begin+=len;
    return ret;
}

int FPBasicIn::getBytesAvailable() {
    return end-begin;
}

int FPBasicIn::getBytesReceived() {
    return begin-orig_begin;
}





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
