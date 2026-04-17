/*
 * VBankFileWriter.cpp -- writes banks into a file
 * by Filip Pizlo, 2004
 */

#include "VBankFileWriter.h"
#include "VBFUtil.h"
#include "VException.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "Adler32.h"
#include <iostream>

using namespace std;
using namespace VBFUtil;

void VBankFileWriter::myWrite(const char *buf,uword32 len) {
    int res=::pwrite(fd,buf,len,(off_t)offset);
    if (res<0) {
        throw VSystemException("In VBankFileWriter::myWrite()");
    }
    if ((uword32)res!=len) {
        // this almost always means no space left on device (at least I
        // don't know what else it could mean)
        throw VSystemException(ENOSPC,"In VBankFileWriter::myWrite()");
    }
    adler=::vbf_adler32(adler,(const unsigned char*)buf,len);
    offset+=len;
}

void VBankFileWriter::myWrite(uint8_t value) {
    myWrite((const char*)&value,1);
}

void VBankFileWriter::myWrite(uword32 value) {
    char buf[4];
    wordToBuf(value,buf);
    myWrite(buf,4);
}

void VBankFileWriter::myWrite(uword64 value) {
    char buf[8];
    wordToBuf(value,buf);
    myWrite(buf,8);
}

void VBankFileWriter::myWrite(word64 offset,const char *buf,uword32 len) {
    int res=::pwrite(fd,buf,len,(off_t)offset);
    if (res<0) {
        throw VSystemException("In VBankFileWriter::myWrite()");
    }
    V_ASSERT((uword32)res==len);    // should always be true
}

void VBankFileWriter::myWrite(word64 offset,uword32 value) {
    char buf[4];
    wordToBuf(value,buf);
    myWrite(offset,buf,4);
}

void VBankFileWriter::myWrite(word64 offset,uword64 value) {
    char buf[8];
    wordToBuf(value,buf);
    myWrite(offset,buf,8);
}

VBankFileWriter::VBankFileWriter(const string &filename,
                                 long run_number,
                                 const vector< bool > &config_mask,
                                 bool keep_index):
    count(0),
    keep_index(keep_index),
    run_number(run_number),
    config_mask(config_mask)
{
    fd=open(filename.c_str(),O_CREAT|O_WRONLY|O_TRUNC,0644);
    if (fd<0) {
        ostringstream buf;
        buf<<"In VBankFileWriter::VBankFileWriter() for \""
           <<filename<<"\"";
        throw VSystemException(buf.str());
    }
    
    adler=::vbf_adler32(0,NULL,0);
    offset=0;
    
    myWrite("VBFF",4);  // magic word
    myWrite((uword32)0);    // version
    myWrite((uword32)run_number);   // run number

    // write the configuration mask at offset 12
    for (unsigned i=0;
         i<32;
         ++i) {
        uint8_t value=0;
        
        for (unsigned j=0;
             j<8;
             ++j) {
            unsigned k=i*8+j;
            // make bit 255 be 0
            if (k==255) {
                continue;
            }
            if (k>=config_mask.size()) {
                continue;
            }
            if (config_mask[k]) {
                value|=(1<<j);
            }
        }
        
        myWrite(value);
    }
    
    myWrite((uword32)0);    // checksum
    myWrite((uword64)0);    // pre-footer size
}

VBankFileWriter::~VBankFileWriter() {
    if (fd>=0) {
        ::close(fd);
    }
}

uword32 VBankFileWriter::getNextIndex() {
    return count;
}

void VBankFileWriter::writePacket(VPacket *packet) {
    if (keep_index) {
        index.push_back(offset);
    }
    
    myWrite("VPCK",4);
    
    uword32 size=8;
    for (VPacket::iterator i=packet->begin();
         i!=packet->end();
         ++i) {
        size+=16;
        size+=i->second->getBankSize();
    }
    myWrite((uword32)size);
    
    for (VPacket::iterator i=packet->begin();
         i!=packet->end();
         ++i) {
        if (i->second==NULL) {
            continue;
        }
        
        myWrite(i->first.getName(),8);
        myWrite(i->second->getBankVersion());
        
        uword32 bank_size=i->second->getBankSize();
        myWrite(bank_size+16);
        
        i->second->writeBankToFile(offset,fd);
        adler=i->second->calcBankAdler(adler);
        offset+=bank_size;
    }
    
    ++count;
}

void VBankFileWriter::writeEmptyPacket() {
    if (keep_index) {
        index.push_back(offset);
    }
    
    myWrite("VPCK",4);
    myWrite((uword32)8);
    
    ++count;
}

void VBankFileWriter::writePacket(uword32 index,
                                  VPacket *packet) {
    if (index<getNextIndex()) {
        throw VBankFileWriterBadIndexException();
    }
    while (index>getNextIndex()) {
        writeEmptyPacket();
    }
    writePacket(packet);
}

uword32 VBankFileWriter::finish(bool store_index) {
    uword64 pre_footer_size=offset;
    
    if (keep_index && store_index) {
        // write num_events
        myWrite((uword32)index.size());
        
        // write the index
        for (vector< uword64 >::iterator i=index.begin();
             i!=index.end();
             ++i) {
            myWrite((uword64)*i);
        }
    }
    
    // write the checksum
    myWrite(44,adler);
    
    // take care of the pre-footer size last
    myWrite(48,pre_footer_size);
    
    ::close(fd);
    fd=-1;
    
    return adler;
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
