/*
 * VArrayEvent.h
 * by Filip Pizlo, 2001, 2002, 2003, 2004
 */

#ifndef V_ARRAY_EVENT_H
#define V_ARRAY_EVENT_H

#include "VDatum.h"
#include "FPIO.h"
#include "VConstants.h"
#include "VException.h"
#include "VBank.h"
#include "VBankBuilder.h"

#include <string>
#include <vector>

class VArrayEventException: public VException {};

class VArrayEventNumberAmbiguousException: public VArrayEventException {
 public:
    VArrayEventNumberAmbiguousException(const std::string &msg="") {
	setStrings("Event number for VArrayEvent is ambiguous",msg);
    }
};

class VArrayEventNumberConflictException: public VArrayEventException {
 public:
    VArrayEventNumberConflictException(const std::string &msg="") {
	setStrings("Conflicting event numbers for VArrayEvent",msg);
    }
};

class VArrayEventRunUnknownException: public VArrayEventException {
 public:
    VArrayEventRunUnknownException(const std::string &msg="") {
	setStrings("Run number for VArrayEvent is not known",msg);
    }
};

class VArrayEventRunConflictException: public VArrayEventException {
 public:
    VArrayEventRunConflictException(const std::string &msg="") {
	setStrings("Run number in VArrayEvent differs from specification",msg);
    }
};

class VArrayEventFullException: public VArrayEventException {
 public:
    VArrayEventFullException(const std::string &msg="") {
	setStrings("The VArrayEvent is full",msg);
    }
};

class VArrayEventBadMagicNumberException: public VArrayEventException {
 public:
    VArrayEventBadMagicNumberException(const std::string &msg="") {
	setStrings("The magic number was not correct",msg);
    }
};

class VArrayEvent: public VBank {
 private:
    std::vector< VEvent* > events;

    long run;
    bool run_set;

    VArrayTrigger *trigger;
    
 protected:
    void setRunButDontConflictImpl(long _run,const char *msg)  ;

    void assertEventNumberImpl(unsigned long ev_num,const char *msg)  ;
        
    void reset();

 public:
    VArrayEvent();
    VArrayEvent(long run);
    virtual ~VArrayEvent();
        
    static uword32 getExpectedMagicNumber() {
	union {
	    uword32 num;
	    char str[4];
	} u;
	u.str[0]='V';
	u.str[1]='A';
	u.str[2]='E';
	u.str[3]='V';
	return u.num;
    }
        
    // bank file functions
    uword32 getBankVersion();
    uword32 getBankSize();
    void writeBankToBuffer(char *buf);
    uword32 calcBankAdler(uword32 adler);
    void writeBankToFile(word64 offset,
			 int fd);

    unsigned getNumEvents() const noexcept { return events.size(); }
    static unsigned getMaxNumEvents()  noexcept { return V_NUM_NODES-1; }
        
    unsigned getNumDatums() const noexcept {
	return getNumEvents()+(hasTrigger()?1:0);
    }
        
    void setRun(long _run);
    void setRunButDontConflict(long _run)  ;
    long getRun()  ;
    bool isRunSet() { return run_set; }
        
    bool hasEventNumber();
    unsigned long getEventNumber()  ;
    void assertEventNumber(unsigned long ev_num)  ;
        
    bool hasTrigger() const noexcept { return trigger!=NULL; }
    VArrayTrigger *getTrigger() const noexcept { return trigger; }
        
    VEvent *getEventAt(unsigned i) const noexcept { return events[i]; }
    VEvent *getEvent(unsigned i) const noexcept { return getEventAt(i); }

    void replaceEventAt(unsigned i,
			VEvent *ev);
        
    // convenience methods that are not intended to be fast!  they will
    // return NULL if the VEvent of VDatum with that node number is not
    // found, or if the node_num is invalid or out of range
    VEvent *getEventByNodeNumber(unsigned node_num);
    VDatum *getDatumByNodeNumber(unsigned node_num);
        
    bool isEmpty() { return !hasTrigger()&&getNumEvents()==0; }
        
    // tells you what telescopes should have contributed based on the info
    // in the array trigger.  if the array trigger is missing, this returns
    // the empty set.
    std::vector< bool > getExpectedTelescopes();
        
    // tells you what telescopes contributed data.
    std::vector< bool > getPresentTelescopes();

    // this function only works for 8 telescopes or less
    bool isSane();

    void setTrigger(VArrayTrigger *_trigger)  ;

    void removeEvent(unsigned node_number);

    void addEvent(VEvent *ev)  ;
        
    void addDatum(VDatum *datum)  ;

    void replaceEvent(VEvent *ev);
        
    // returns true if adding the datum will:
    // -> not result in any exceptions
    // -> not result in an existing trigger being overwritten
    bool canAddDatum(VDatum *datum);

    void setEventsAndTrigger(VArrayTrigger *_trigger,
			     VEvent **evs,
			     unsigned evn)  ;
        
    // this method is slow. don't use it when performance is essential.
    bool isEqualTo(VArrayEvent *ae);
        
    // useful for debugging...
    std::string getSummary();
        
};

const VBankName &VGetArrayEventBankName();
VBankBuilder *VGetArrayEventBankBuilder();

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
