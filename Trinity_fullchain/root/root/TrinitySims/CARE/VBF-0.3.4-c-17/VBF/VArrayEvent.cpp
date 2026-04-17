/*
 * VArrayEvent.cpp -- array event module
 * by Filip Pizlo, 2001, 2002, 2003, 2004, 2005
 */

#include "VArrayEvent.h"

#include "FPBasics.h"
#include "FPAdler.h"
#include "FPFDWrite.h"
#include "FPFDRead.h"

#include "VConfigMaskUtil.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

using namespace std;

static VArrayEvent *buildBankFrom(long run_number,
                                  uword32 event_number,
				  uword32 version,
                                  uword32 size,
                                  FPInputBase *in) {
    V_ASSERT(size>0);
    VDatum::Version versionEnum;
    switch (version) {
    case 0:
	versionEnum=VDatum::AUG_2004;
	break;
    case 1:
	versionEnum=VDatum::AUG_2005;
	break;
    default:
	throw VBankBuilderVersionException("While building VArrayEvent");
    }
    VArrayEvent *result=new VArrayEvent(run_number);
    while (in->getBytesReceived()<(int)size) {
        result->addDatum(VDatumParser::buildFrom(in,versionEnum));
    }
    if (result->getEventNumber()!=event_number) {
        ostringstream buf;
        buf<<"The bank that we're reading is supposed to have "
             "event number "<<event_number<<" but the array event "
             "we read has number "<<result->getEventNumber();
        throw VArrayEventNumberConflictException(buf.str());
    }
    return result;
}

class VArrayEventBankBuilder:
    public VBankBuilder
{
 public:
        
    VBank *readBankFromBuffer(long run_number,
			      uword32 event_number,
			      const VBankName &name,
			      uword32 version,
			      uword32 size,
			      const char *buf) {
	FPBasicIn in(buf,size);
	return buildBankFrom(run_number,event_number,version,size,&in);
    }
        
    VBank *readBankFromFile(long run_number,
			    uword32 event_number,
			    const VBankName &name,
			    uword32 version,
			    uword32 size,
			    off_t offset,
			    int fd) {
	FPFDRead in(fd,offset);
	return buildBankFrom(run_number,event_number,version,size,&in);
    }
        
};

static VBankName bank_name("CoreVAEV");
static VArrayEventBankBuilder bank_builder;

const VBankName &VGetArrayEventBankName() {
    return bank_name;
}

VBankBuilder *VGetArrayEventBankBuilder() {
    return &bank_builder;
}

VArrayEvent::VArrayEvent():
    run(-1),
    run_set(false),
    trigger(NULL)
{
}

VArrayEvent::VArrayEvent(long _run):
    run(_run),
    run_set(true),
    trigger(NULL)
{
}

VArrayEvent::~VArrayEvent() {
    if (trigger!=NULL) {
        delete trigger;
    }
    
    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        delete *i;
    }
}

VEvent *VArrayEvent::getEventByNodeNumber(unsigned node_num) {
    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        if ((*i)->getNodeNumber()==node_num) {
            return *i;
        }
    }
    return NULL;
}

VDatum *VArrayEvent::getDatumByNodeNumber(unsigned node_num) {
    if (node_num==V_ARRAY_TRIGGER_NODE) {
        return getTrigger();
    }
    
    return getEventByNodeNumber(node_num);
}

void VArrayEvent::setRun(long _run) {
    run=_run;
    run_set=true;
}

void VArrayEvent::setRunButDontConflictImpl(long _run,const char *msg)  {
    if (run_set&&run!=_run) {
        throw VArrayEventRunConflictException(msg);
    }
    setRun(_run);
}

void VArrayEvent::setRunButDontConflict(long _run)  {
    setRunButDontConflictImpl(_run,"In setRunButDontConflict()");
}

long VArrayEvent::getRun()  {
    if (!run_set) {
        throw VArrayEventRunUnknownException("In getRun()");
    }
    return run;
}

void VArrayEvent::replaceEventAt(unsigned i,
				 VEvent *ev) {
    delete events[i];
    events[i]=ev;
}

void VArrayEvent::setTrigger(VArrayTrigger *_trigger)  {
    assertEventNumberImpl(_trigger->getEventNumber(),"In setTrigger()");
    
    setRunButDontConflictImpl(_trigger->getRunNumber(),
                              "In VArrayTrigger");
    
    if (trigger!=NULL) {
        delete trigger;
    }
    
    trigger=_trigger;
}

void VArrayEvent::removeEvent(unsigned node_num) {
    for (unsigned i=0;
	 i<events.size();
	 ++i) {
	if (events[i]->getNodeNumber()==node_num) {
	    delete events[i];
	    events[i]=events.back();
	    events.pop_back();
	    return;
	}
    }
}

void VArrayEvent::addEvent(VEvent *ev)  {
    if (events.size()==getMaxNumEvents()) {
        throw VArrayEventFullException("In call to VArrayEvent::addEvent()");
    }
    
    assertEventNumberImpl(ev->getEventNumber(),"In addEvent()");
    
    events.push_back(ev);
}

void VArrayEvent::addDatum(VDatum *datum)  {
    if (datum->getNodeNumber()==V_ARRAY_TRIGGER_NODE) {
        if (hasTrigger()) {
            throw VArrayEventFullException(
                "In addDatum(), trying to add trigger when we already "
                "have one");
        }
        setTrigger((VArrayTrigger*)datum);
    } else {
        addEvent((VEvent*)datum);
    }
}

bool VArrayEvent::canAddDatum(VDatum *datum) {
    if (datum->getNodeNumber()==V_ARRAY_TRIGGER_NODE) {
        VArrayTrigger *trig=(VArrayTrigger*)datum;
        return !hasTrigger()&&
               (!isRunSet()||(long)trig->getRunNumber()==getRun())&&
               (!hasEventNumber()||
                getEventNumber()==trig->getEventNumber());
    } else {
        return getNumEvents()<V_NUM_TELESCOPES&&
               (!hasEventNumber()||
                getEventNumber()==datum->getEventNumber());
    }
}

void VArrayEvent::setEventsAndTrigger(VArrayTrigger *_trigger,
                                      VEvent **evs,
                                      unsigned evn)  {
    if (evn>V_NUM_TELESCOPES) {
        throw VArrayEventFullException(
            "Too many events in call to VArrayEvent::setEventsAndTrigger()");
    }
    
    for (unsigned i=0;i<evn;i++) {
        if (evs[i]->getEventNumber()!=_trigger->getEventNumber()) {
            throw VArrayEventNumberConflictException(
                "In setEventsAndTrigger()");
        }
    }
    
    setRunButDontConflictImpl(_trigger->getRunNumber(),"In VArrayTrigger");
    
    if (trigger!=NULL) {
        delete trigger;
    }
    
    trigger=_trigger;
    
    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        delete *i;
    }
    
    events.clear();
    
    for (unsigned i=0;i<evn;i++) {
        events.push_back(evs[i]);
    }
}

void VArrayEvent::reset() {
    if (trigger!=NULL) {
        delete trigger;
        trigger=NULL;
    }
    
    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        delete *i;
        *i=NULL;
    }
    
    run=-1;
    run_set=false;
}

uword32 VArrayEvent::getBankVersion() {
    VDatum::Version v=VDatum::AUG_2004; // make gcc happy
    bool hasV=false;
    for (unsigned i=events.size();i-->0;) {
	VDatum::Version thisV=events[i]->getVersion();
	if (hasV) {
	    V_ASSERT(v==thisV);
	} else {
	    v=thisV;
	    hasV=true;
	}
    }
    if (trigger!=NULL) {
	VDatum::Version thisV=trigger->getVersion();
	if (hasV) {
	    V_ASSERT(v==thisV);
	} else {
	    v=thisV;
	    hasV=true;
	}
    }
    V_ASSERT(hasV);
    switch (v) {
    case VDatum::AUG_2004: return 0;
    case VDatum::AUG_2005: return 1;
    default: V_FAIL("bad version");
    }
}

uword32 VArrayEvent::getBankSize() {
    uword32 result=0;
    if (trigger!=NULL) {
        result+=trigger->getSize();
    }
    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        result+=(*i)->getSize();
    }
    return result;
}

void VArrayEvent::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    if (trigger!=NULL) {
        trigger->writeTo(&out);
    }
    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        (*i)->writeTo(&out);
    }
}

uword32 VArrayEvent::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    if (trigger!=NULL) {
        trigger->writeTo(&out);
    }
    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        (*i)->writeTo(&out);
    }
    return out.getAdler();
}

void VArrayEvent::writeBankToFile(word64 offset,
                                  int fd) {
    FPFDWrite out(fd,offset);
    if (trigger!=NULL) {
        trigger->writeTo(&out);
    }
    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        (*i)->writeTo(&out);
    }
}

bool VArrayEvent::hasEventNumber() {
    return trigger!=NULL||!events.empty();
}

unsigned long VArrayEvent::getEventNumber()  {
    if (trigger==NULL) {
        if (!events.empty()) {
            return events.front()->getEventNumber();
        }
        throw VArrayEventNumberAmbiguousException("Because there are no events or "
						  "triggers associated with this "
						  "array event");
    }
    return trigger->getEventNumber();
}

void VArrayEvent::assertEventNumberImpl(unsigned long ev_num,
                                        const char *msg)  {
    if (!hasEventNumber()) {
        return;
    }
    
    if (getEventNumber()!=ev_num) {
        throw VArrayEventNumberConflictException(msg);
    }
}

void VArrayEvent::assertEventNumber(unsigned long ev_num)  {
    assertEventNumberImpl(ev_num,"In assertEventNumber()");
}

std::vector< bool > VArrayEvent::getExpectedTelescopes() {
    std::vector< bool > result(V_NUM_NODES-1,false);
    if (hasTrigger()) {
        VArrayTrigger *at=getTrigger();
        for (unsigned i=0;
             i<at->getNumTriggerTelescopes();
             ++i) {
            if (at->getTriggerTelescopeId(i)<V_ARRAY_TRIGGER_NODE) {
                result[at->getTriggerTelescopeId(i)]=true;
            } /* else I should probably throw some sort of exception */
        }
    }
    return result;
}

std::vector< bool > VArrayEvent::getPresentTelescopes() {
    std::vector< bool > result(V_NUM_NODES-1,false);
    for (unsigned i=0;
         i<getNumEvents();
         ++i) {
        result[getEvent(i)->getNodeNumber()]=true;
    }
    return result;
}

bool VArrayEvent::isSane() {
    return trigger!=NULL&&
           VConfigMaskUtil::masksEqual(getExpectedTelescopes(),
                                       getPresentTelescopes());
}

// useful function for isEqualTo
static bool event_less_than(VEvent *a,VEvent *b) {
    return a->getNodeNumber()<b->getNodeNumber();
}

// useful typedef
typedef vector< VEvent* > event_vector;

bool VArrayEvent::isEqualTo(VArrayEvent *ae) {
    if (isRunSet()!=ae->isRunSet()) {
        return false;
    }
    
    if (isRunSet()) {
        if (getRun()!=ae->getRun()) {
            return false;
        }
    }
    
    if (hasTrigger()!=ae->hasTrigger()) {
        return false;
    }
    
    if (hasTrigger()) {
        if (!getTrigger()->isEqualTo(ae->getTrigger())) {
            return false;
        }
    }
    
    if (getNumEvents()!=ae->getNumEvents()) {
        return false;
    }
    
    // put both lists of events into vectors, sort them, and
    // then compare them.
    event_vector a,b;

    for (vector< VEvent* >::iterator
         i=events.begin();
         i!=events.end();
         ++i) {
        a.push_back(*i);
    }
    
    for (vector< VEvent* >::iterator
         i=ae->events.begin();
         i!=ae->events.end();
         ++i) {
        b.push_back(*i);
    }
    
    sort(a.begin(),a.end(),event_less_than);
    sort(b.begin(),b.end(),event_less_than);
    
    event_vector::iterator i,j;
    for (i=a.begin(), j=b.begin();
         i!=a.end() && j!=b.end();
         ++i, ++j)
    {
        if (!(*i)->isEqualTo(*j)) {
            return false;
        }
    }
    
    return true;
}

string VArrayEvent::getSummary() {
    ostringstream buf;
    buf<<"(Trigger? "<<(hasTrigger()?"Yes":"No")<<"; "
       <<"# Events: "<<getNumEvents()<<"; "
       <<"Run Num? "<<(isRunSet()?"Yes":"No")<<"; "
       <<"Event Num? "<<(hasEventNumber()?"Yes":"No");
    if (isRunSet()) {
        buf<<"; Run Num: "<<getRun();
    }
    if (hasEventNumber()) {
        buf<<"; Event Num: "<<getEventNumber();
    }
    buf<<")";
    return buf.str();
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
