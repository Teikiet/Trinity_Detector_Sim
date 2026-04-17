/*
 * VBankName.h -- the name of a bank
 * by Filip Pizlo, 2004
 */

#ifndef V_BANK_NAME_H
#define V_BANK_NAME_H

#include "VException.h"
#include <string>
#include <string.h>
#include "Words.h"

class VBankNameException: public VException {};

class VBankNameTooLongException: public VBankNameException {
    public:
        VBankNameTooLongException(const std::string &msg="") {
            setStrings("The given bank name is too long (must be at most "
                       "8 bytes)",msg);
        }
};

class VBankName {
    private:
        
        char name[8];
        
    public:
        
        // WARNING: this does NOT take a C-string, but rather an 8-int8_t long
        // buffer!
        VBankName(const char *name) {
            memcpy(this->name,name,8);
        } 
	        
        VBankName(const std::string &name) {
            if (name.length()>8) {
                throw VBankNameTooLongException();
            }
            memcpy(this->name,name.data(),name.length());
            bzero(this->name+name.length(),8-name.length());
        }
        
        // NOTE: this is not a C-string!!
        const char *getName() const noexcept {
            return name;
        }
        
        std::string getNameAsString() const noexcept {
            return std::string(name,8);
        }
        
        bool operator==(const VBankName &other) const noexcept {
            return !bcmp(name,other.name,8);
        }
        
        bool operator<(const VBankName &other) const noexcept {
            return memcmp(name,other.name,8)<0;
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
