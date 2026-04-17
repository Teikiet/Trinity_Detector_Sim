/*
 * VKascadeSimulationHeader.h -- a class that stores specific Kascade 
 * simulation data in the VBF event KASCADE Simulation bank
 * by Glenn Sembroski
 *
 */

#ifndef V_KASCADE_SIMULATION_HEADER_H
#define V_KASCADE_SIMULATION_HEADER_H

#include "VBank.h"
#include "VBankBuilder.h"

#include "FPIO.h"
#include "VException.h"

#include <string>
#include <vector>
#include "Adler32.h"



// make it a struct so that everything is public.  that really makes most
// sense since otherwise it would just end up having a bunch of getters and
// setters.

// *********************************************************************
// Do not Inherit from VSimulationHeader. This is for the KASCADE specific
// stuff only. Write(and read) standard Simulation banks and KASCADESimulaiton
// banks sperately
// *********************************************************************


enum VKascadeGridTypes {SQUAREGRID=0,NORTHSOUTHGRID=1,EASTWESTGRID=2};

struct VKascadeSimulationHeader:public VBank
{
  // ************************************************************************
  // FP: I tacked this  field onto here; I figured it  won't hurt!
  // (note: if you're creating this structure in order to save it in a
  // VBF file, you DO NOT have to set this field; in fact, even if you
  // do set it, it will be ignored!  however, this field will be
  // automatically set for you when you read a VKascadeSimulationHeader bank 
  // from a VBF file.)
  long fRunNumber;
  // *********************************************************************
  // Below are the Kascade specific parameters that go with each shower run.
  // *********************************************************************
  //Special stuff for Kascade
  uword32 fCORSIKAParticleID;
  float   fEnergyGeV;
  uword32 fShowerID;
  float   fXAreaWidthM;	// Grid width in x direction in meters.
  float   fYAreaWidthM;	// Grid width in y direction in merters.
  uword32 fNorthSouthGrid;  // Flag grid type, see enum above

  VKascadeSimulationHeader(){};
  VKascadeSimulationHeader(uword32 CORSIKAParticleID, float EnergyGeV,
			   uword32 ShowerID, float XAreaWidthM, 
			   float YAreaWidthM, uword32 NorthSouthGrid)
  {
    // ***********************************************************************
    // Kascade stuff
    // ***********************************************************************
    fCORSIKAParticleID=CORSIKAParticleID;
    fEnergyGeV=EnergyGeV;
    fShowerID=ShowerID;
    fXAreaWidthM=XAreaWidthM;
    fYAreaWidthM=YAreaWidthM;
    fNorthSouthGrid=NorthSouthGrid;
}

  virtual ~VKascadeSimulationHeader() {};
    
  // bank operations - you probably don't want to use these directly.
  uword32 getBankVersion();
  uword32 getBankSize();
  void writeBankToBuffer(char *buf);
  uword32 calcBankAdler(uword32 adler);
  void writeBankToFile(word64 offset,
                         int fd);
    
  // lower-level writing operation
  void writeTo(FPOutputBase *out);
};

const VBankName &VGetKascadeSimulationHeaderBankName();
VBankBuilder *VGetKascadeSimulationHeaderBankBuilder();

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
