/*
 * VPacket.h -- a packet is a per-event-number entity that contains named banks
 *              of data
 * by Filip Pizlo, 2004, 2005
 */

#ifndef V_PACKET_H
#define V_PACKET_H

#include "VException.h"
#include "VArrayEvent.h"
#include "VSimulationHeader.h"
#include "VSimulationData.h"
#include "VKascadeSimulationHeader.h"
#include "VKascadeSimulationData.h"
#include "VCorsikaSimulationData.h"
#include "VChiLASimulationHeader.h"
#include "VChiLASimulationData.h"
#include "VEventOverflow.h"
#include <map>

#include "VBankName.h"
#include "VBank.h"
#include <iostream>

class VPacketException: public VException {};

class VPacketDoesNotContainGivenBankNameException: public VPacketException {
    public:
        VPacketDoesNotContainGivenBankNameException(
                const std::string &msg="") {
            setStrings("The given packet does not contain any banks by the "
                       "given name",msg);
        }
};

class VPacketBadTypeForBankNameException: public VPacketException {
    public:
        VPacketBadTypeForBankNameException(const std::string &msg="") {
            setStrings("The bank with the given name does not have the "
                       "requested type",msg);
        }
};

class VPacket {
    private:
        
        VPacket(const VPacket& other) {}
        void operator=(const VPacket& other) {}
        
        typedef std::map< VBankName, VBank* > bank_map;
        
        bank_map banks;
        
    public:
        
        typedef bank_map::const_iterator iterator;
        
        VPacket() {}
        virtual ~VPacket() {
            for (bank_map::iterator
                 i=banks.begin();
                 i!=banks.end();
                 ++i) {
		delete i->second;
             }
        }
        
        bool empty() {
            return banks.empty();
        }
        
        unsigned size() {
            return banks.size();
        }
        
        bool has(const VBankName &name) const {
            return banks.count(name);
        }
        
        bool hasArrayEvent() const {
            return has(VGetArrayEventBankName());
        }
        
        bool hasSimulationHeader() const {
            return has(VGetSimulationHeaderBankName());
        }

        bool hasSimulationData() const {
            return has(VGetSimulationDataBankName());
        }

	bool hasCorsikaSimulationData() const {
            return has(VGetCorsikaSimulationDataBankName());
        }
	
	bool hasEventOverflow() const {
	    return has(VGetEventOverflowBankName());
	}
        
        void put(const VBankName &name,
                 VBank *bank) {
            bank_map::iterator i=banks.find(name);
           
            if (i==banks.end()) {
                banks[name]=bank;
            } else {
		delete i->second;
                i->second=bank;
            }
        }
        
        void putArrayEvent(VArrayEvent *ae) {
            put(VGetArrayEventBankName(),ae);
        }
        
        void putSimulationHeader(VSimulationHeader *sd) {
            put(VGetSimulationHeaderBankName(),sd);
        }

        void putSimulationData(VSimulationData *sd) {
            put(VGetSimulationDataBankName(),sd);
        }

	void putCorsikaSimulationData(VCorsikaSimulationData *sd) {
            put(VGetCorsikaSimulationDataBankName(),sd);
        }
	
	void putEventOverflow(VEventOverflow *eo) {
	    put(VGetEventOverflowBankName(),eo);
	}
        
        void skipBank(const VBankName &name) {
            put(name,NULL);
        }
        
        template< typename T >
        T *get(const VBankName &name) const {
            bank_map::const_iterator i=banks.find(name);
            if (i==banks.end()) {
                throw VPacketDoesNotContainGivenBankNameException();
            }
            
            T *result=dynamic_cast<T*>(i->second);
            if (result==NULL) {
                throw VPacketBadTypeForBankNameException();
            }
            
            return result;
        }
        
        VArrayEvent *getArrayEvent() const {
            return get< VArrayEvent >(VGetArrayEventBankName());
        }
        
        VSimulationHeader *getSimulationHeader() const {
            return get< VSimulationHeader >(VGetSimulationHeaderBankName());
        }

        VSimulationData *getSimulationData() const {
            return get< VSimulationData >(VGetSimulationDataBankName());
        }

	VCorsikaSimulationData *getCorsikaSimulationData() const {
            return get< VCorsikaSimulationData >(VGetCorsikaSimulationDataBankName());
        }
	
	VEventOverflow *getEventOverflow() const {
	    return get< VEventOverflow >(VGetEventOverflowBankName());
	}
        
        void remove(const VBankName &name) {
            delete banks[name];
            banks.erase(name);
        }
        
        iterator begin() {
            return banks.begin();
        }
        
        iterator end() {
            return banks.end();
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
