/*
 * VEventType.h
 * by Filip Pizlo, 2005
 */

#ifndef V_EVENT_TYPE_H
#define V_EVENT_TYPE_H

#include "VException.h"

class VEventTypeException: public VException {
 public:
    VEventTypeException(const std::string &msg="") {
	setStrings("Event type error",msg);
    }
};

struct VEventType {
    enum TriggerType {
	L2_TRIGGER,
	HIGH_MULT_TRIGGER,
	NEW_PHYS_TRIGGER,
	CAL_TRIGGER,
	PED_TRIGGER
    };
    
    enum CalibrationType {
	NOT_CALIBRATION,
	OPTICAL_CALIBRATION,
	CHARGE_CALIBRATION
    };
    
    TriggerType trigger;
    bool xtra_samples;
    bool force_full_mode;
    CalibrationType calibration;
    bool has_error;
    
    VEventType() {}
    
    VEventType(TriggerType trigger,
	       bool xtra_samples,
	       bool force_full_mode,
	       CalibrationType calibration,
	       bool has_error=false):
	trigger(trigger),
	xtra_samples(xtra_samples),
	force_full_mode(force_full_mode),
	calibration(calibration),
	has_error(has_error)
    {}
    
    ~VEventType() {}
    
    void setOldStyleCode(unsigned code) {
	using namespace std;
	has_error=false;
	switch (code) {
	case 1:
	    trigger=L2_TRIGGER;
	    xtra_samples=false;
	    force_full_mode=false;
	    calibration=NOT_CALIBRATION;
	    break;
	case 2:
	    trigger=PED_TRIGGER;
	    xtra_samples=false;
	    force_full_mode=true;
	    calibration=NOT_CALIBRATION;
	    break;
	case 3:
	    trigger=L2_TRIGGER;
	    xtra_samples=false;
	    force_full_mode=false;
	    calibration=OPTICAL_CALIBRATION;
	    break;
	case 4:
	    trigger=PED_TRIGGER;
	    xtra_samples=false;
	    force_full_mode=true;
	    calibration=NOT_CALIBRATION;
	    break;
	default:
	    trigger=L2_TRIGGER;
	    xtra_samples=false;
	    force_full_mode=false;
	    calibration=NOT_CALIBRATION;
	    has_error=true;
	    break;
	}
    }

    void setNewStyleCode(unsigned code) {
	using namespace std;
	trigger=L2_TRIGGER;
	xtra_samples=false;
	force_full_mode=false;
	calibration=NOT_CALIBRATION;
	if (code<1 || code>14) {
	    has_error=true;
	    return;
	}
	switch (code) {
	case 4:
	case 5:
	case 6:
	    trigger=HIGH_MULT_TRIGGER;
	    break;
	case 7:
	case 8:
	case 9:
	    trigger=NEW_PHYS_TRIGGER;
	    break;
	case 10:
	    trigger=PED_TRIGGER;
	    break;
	case 11:
	case 13:
	    trigger=CAL_TRIGGER;
	    break;
	default:
	    break;
	}
	switch (code) {
	case 3:
	case 6:
	case 9:
	    force_full_mode=true;
	case 2:
	case 5:
	case 8:
	    xtra_samples=true;
	    break;
	default:
	    break;
	}
	switch (code) {
	case 11:
	case 12:
	    calibration=OPTICAL_CALIBRATION;
	    break;
	case 13:
	case 14:
	    calibration=CHARGE_CALIBRATION;
	    break;
	default:
	    break;
	}
    }
    
    uint8_t getBestOldStyleCode() const {
	if (trigger==PED_TRIGGER) {
	    return 2;
	}
	if (calibration!=NOT_CALIBRATION) {
	    return 3;
	}
	return 1;
    }
    
    uint8_t getBestNewStyleCode() const {
	switch (trigger) {
	case L2_TRIGGER:
	    if (calibration==OPTICAL_CALIBRATION) {
		return 12;
	    } else if (calibration==CHARGE_CALIBRATION) {
		return 14;
	    } else {
		if (xtra_samples) {
		    if (force_full_mode) {
			return 3;
		    } else {
			return 2;
		    }
		} else {
		    return 1;
		}
	    }
	case HIGH_MULT_TRIGGER:
	    if (xtra_samples) {
		if (force_full_mode) {
		    return 6;
		} else {
		    return 5;
		}
	    } else {
		return 4;
	    }
	case NEW_PHYS_TRIGGER:
	    if (xtra_samples) {
		if (force_full_mode) {
		    return 9;
		} else {
		    return 8;
		}
	    } else {
		return 7;
	    }
	case CAL_TRIGGER:
	    if (calibration==CHARGE_CALIBRATION) {
		return 13;
	    } else {
		return 11;
	    }
	case PED_TRIGGER:
	    return 10;
	default:
	    V_FAIL("bad value for trigger type");
	}
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
