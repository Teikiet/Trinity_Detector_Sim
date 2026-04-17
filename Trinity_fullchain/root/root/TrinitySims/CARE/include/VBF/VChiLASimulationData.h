/*
 * VChiLASimulationData.h -- a class that stores specific ChiLA 
 * simulation data in the VBF event 
 * by Matthew Wood and Stephen Fegan
 *
 */

#ifndef V_CHILA_SIMULATION_DATA_H
#define V_CHILA_SIMULATION_DATA_H

#define V_CHILA_SIMULATION_DATA_VERSION 1

#include "VBank.h"
#include "VBankBuilder.h"

#include "FPIO.h"
#include "VException.h"

#include <string>
#include "Adler32.h"


// make it a struct so that everything is public.  that really makes most
// sense since otherwise it would just end up having a bunch of getters and
// setters.

struct VChiLASimulationData:public VBank
{
  // FP: I tacked these 2  fields onto here; I figured it  won't hurt!
  // (note: if you're creating this structure in order to save it in a
  // VBF file, you DO NOT have to set these fields; in fact, even if you
  // do set them,they will be ignored!  however, these fields will be
  // automatically set for you when you read a VSimulationData bank from
  // a VBF file.)
  long fRunNumber;
  uword32 fEventNumber;           // VBF event number to match with data banks
 
  // *********************************************************************
  // Below are the ChiLA specific parameters that go with each event.
  // *********************************************************************
  uword32 fSimParamTableID;      // Index of SimParamTable in
				 // VChiLASimulationHeader
  uword32 fElectronicsID;        // Index of Electronics Configuration in 
                                 // VChilaSimulationHeader
  uword32 fTableIndex;           // Index of this event in its DB table

  // *********************************************************************
  // The parameters fAomega and fAomegaVar allow the calculation of
  // the effective area and its statistical error using the formulas
  // below where i is the event index and 'Sum' indicates the sum over
  // all events which pass cuts:
  //
  // A_eff(E)      = Sum_i fAomega_i
  // Err(A_eff(E)) = sqrt(Sum_i fAomega_i^2 - (Sum_i fAomegaVar_i)^2)
  // *********************************************************************

  float   fAomega;               //Area-solid angle contribution of this event
  float   fAomegaVar;            //Parameter used to calculate
				 //statistical error on fAomega

  VChiLASimulationData(){};
  VChiLASimulationData(uword32 SimParamTableID, uword32 ElectronicsID,
		       uword32 TableIndex, float Aomega, float AomegaVar)
  {    
    // ***********************************************************************
    // ChiLA stuff
    // ***********************************************************************
    fSimParamTableID = SimParamTableID;
    fElectronicsID = ElectronicsID;
    fTableIndex = TableIndex;
    fAomega = Aomega;
    fAomegaVar = AomegaVar;

  };
  
  virtual ~VChiLASimulationData() {};
    
  // bank operations - you probably don't want to use these directly.
  uword32 getBankVersion();
  uword32 getBankSize();
  void writeBankToBuffer(char *buf);
  uword32 calcBankAdler(uword32 adler);
  void writeBankToFile(word64 offset,
                         int fd);
    
  // lower-level writing operation
  void writeTo(FPOutputBase *out);
  // Make a new one
  VChiLASimulationData* copyChiLASimData();
};

const VBankName &VGetChiLASimulationDataBankName();
VBankBuilder *VGetChiLASimulationDataBankBuilder();

#endif // V_CHILA_SIMULATION_DATA_H

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
