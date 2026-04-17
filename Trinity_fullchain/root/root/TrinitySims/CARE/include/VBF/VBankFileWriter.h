/*
 * VBankFileWriter.h -- writes banks into a file
 * by Filip Pizlo, 2004
 */

#ifndef V_BANK_FILE_WRITER_H
#define V_BANK_FILE_WRITER_H

#include "VException.h"
#include "VPacket.h"

class VBankFileWriterException: public VException {};

class VBankFileWriterBadIndexException: public VBankFileWriterException {
    public:
        VBankFileWriterBadIndexException(const std::string &msg="") {
            setStrings("Attempt to write a packet with an index that is not "
                       "greater than the index of the preceding packet",msg);
        }
};

class VBankFileWriter {
    private:
        
        VBankFileWriter(const VBankFileWriter& other) {}
        void operator=(const VBankFileWriter &other) {}
        
        int fd;
        word64 offset;
        uword32 count;
        uword32 adler;
        bool keep_index;
        std::vector< uword64 > index;
        
        // kept around for convenience
        long run_number;
        std::vector< bool > config_mask;
        
        // high-level write functions.  these write to the current file
        // pointer location, keep the offset up to date, and manage the
        // running adler checksum.
        void myWrite(const char *buf,uword32 len);
        void myWrite(uint8_t value);
        void myWrite(uword32 value);
        void myWrite(uword64 value);
        
        // low-level write functions.  these write to the specified offset,
        // do not side-effect either the file pointer or the offset field,
        // and do not do anything to the adler checksum.
        void myWrite(word64 offset,const char *buf,uword32 len);
        void myWrite(word64 offset,uword32 value);
        void myWrite(word64 offset,uword64 value);
        
    public:
        
        VBankFileWriter(const std::string &filename,
                        long run_number,
                        const std::vector< bool > &config_mask,
                        bool keep_index=true);
        
        virtual ~VBankFileWriter();
        
        long getRunNumber() const noexcept {
            return run_number;
        }
        
        const std::vector< bool > &getConfigMask() const noexcept {
            return config_mask;
        }
        
        // get the index of the next packet to write
        uword32 getNextIndex();
        
        // write another packet.
        void writePacket(VPacket *packet);
        
        // write an empty packet
        void writeEmptyPacket();
        
        // write a packet with the given index; that index must be greater
        // than or equal to getNextIndex()
        void writePacket(uword32 index,
                         VPacket *packet);
        
        // finishes up and closes the file.  you cannot use this object
        // after calling the finish method.  this method returns the final
        // adler checksum.  not calling this method and destroying this
        // object will lead to a file that has neither a checksum nor an
        // index.  calling this method with false will result in a file
        // that has a checksum but no index.  calling this method with
        // true will not result in an index being appended if you passed
        // false for keep_index to the constructor.
        uword32 finish(bool store_index=true);
        
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
