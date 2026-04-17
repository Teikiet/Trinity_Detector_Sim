//-*-mode:c++; mode:font-lock;-*-
/*
 * VKascadeSimulationData.h -- a class that stores specific Kascade 
 * simulation data in the VBF event Kascade Simulation Data bank
 * by Glenn Sembroski
 *
 */

#ifndef V_KASCADE_SIMULATION_DATA_H
#define V_KASCADE_SIMULATION_DATA_H

#define V_KASCADE_SIMULATION_DATA_VERSION 1

#include "VBank.h"
#include "VBankBuilder.h"

#include "FPIO.h"
#include "VException.h"

#include <string>
#include "Adler32.h"


// make it a struct so that everything is public.  that really makes most
// sense since otherwise it would just end up having a bunch of getters and
// setters.

struct VKascadeSimulationData:public VBank
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
  // Below are the Kascade specific parameters that go with each event.
  // *********************************************************************
  word32  fNx;                   //Mount Grid coordinates.
  word32  fNy;
  word32  fDirectionIndex;       //Mount direction index
  float   fEmissionAltitudeM;    //Altitude in meters of Max Chrenkov emission.
  float   fEmissionAltitudeSigma;//Sigma of max Altitude of emission in meters
  float   fMuonRatio;            //Fractrion of pe's due to muons in image.
  float   fAomega;               //Area-solid angle contribution of this event
  float   fRelTelTriggerTimeNS;  //Event trigger time rleative to orig
  float   fDifferentialRatePerEventHz;  //Contribution to differential rate
  float   fIntegralRatePerEventHz;      //Contribution to integral rate
  uword32 fShowerID;             //ID Of shower. Used in makeEffectiveArea for
                                 //error calculation
  VKascadeSimulationData(){};
  VKascadeSimulationData(word32 Nx, word32 Ny, word32 DirectionIndex,
			 float EmissionAltitudeM, float EmissionAltitudeSigma,
			 float MuonRatio, float Aomega,
			 float RelTelTriggerTimeNS, 
			 float DifferentialRatePerEventHz,
			 float IntegralRatePerEventHz, uword32 ShowerID)

  {    
    // ***********************************************************************
    // Kascade stuff
    // ***********************************************************************
    fNx=Nx;
    fNy=Ny; 
    fDirectionIndex=DirectionIndex;
    fEmissionAltitudeM=EmissionAltitudeM; 
    fEmissionAltitudeSigma=EmissionAltitudeSigma;
    fMuonRatio=MuonRatio; 
    fAomega=Aomega;
    fRelTelTriggerTimeNS=RelTelTriggerTimeNS;
    fDifferentialRatePerEventHz=DifferentialRatePerEventHz;
    fIntegralRatePerEventHz=IntegralRatePerEventHz;
    fShowerID=ShowerID;
};
  
  virtual ~VKascadeSimulationData() {};
    
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
  VKascadeSimulationData* copyKascadeSimData();
};

const VBankName &VGetKascadeSimulationDataBankName();
VBankBuilder *VGetKascadeSimulationDataBankBuilder();

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
