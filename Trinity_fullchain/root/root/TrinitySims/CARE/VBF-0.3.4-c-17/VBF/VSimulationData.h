/*
 * VSimulationData.h -- a bank that stores simulation data
 * by Filip Pizlo, 2004
 *
 * This bank is based on documentation received from VVV.
 * Modified by Glenn Sembroski
 */

#ifndef V_SIMULATION_DATA_H
#define V_SIMULATION_DATA_H

#define V_SIMULATION_DATA_VERSION 2

#include "VBank.h"
#include "VBankBuilder.h"

#include "FPIO.h"
#include "VException.h"

#include <string>
#include <cmath>
#include "Adler32.h"

// make it a struct so that everything is public.  that really makes most
// sense since otherwise it would just end up having a bunch of getters and
// setters.
struct VSimulationData:public VBank
{

  // ************************************************************************
  // This class is the base class for the simulation bank. For a specific 
  // simulation (Ex. Kascade, GrISU etc), it is inherited by
  // that specific simulation's simulation bank class.
  // ************************************************************************
  // FP: I tacked these 2  fields onto here; I figured it  won't hurt!
  // (note: if you're creating this structure in order to save it in a
  // VBF file, you DO NOT have to set these fields; in fact, even if you
  // do set them,they will be ignored!  however, these fields will be
  // automatically set for you when you read a VSimulationData bank from
  // a VBF file.)
  long fRunNumber;
  uword32 fEventNumber;           // VBF event number to match with data banks
  
  // *************************************************
  //Below is what gets written to simulation bank
  // *************************************************
  uword32 fCORSIKAParticleID;     // CORSIKA ID for the particle. See below 
  
  float fEnergyGeV;               // Energy of the primary in GeV
  
  float fObservationZenithDeg;    // Observation zenith in degrees
  float fObservationAzimuthDeg;   // Observation azimuth in degrees
  
  float fPrimaryZenithDeg;        // Particle zenith in degrees   (for off-axis
  float fPrimaryAzimuthDeg;       // Particle azimuth in degrees  observations)

  float fRefZenithDeg;            // Reference zenith in degrees   
  float fRefAzimuthDeg;           // Reference azimuth in degrees  
  float fRefPositionAngleDeg;     // Reference orientation in degrees

  float fCoreEastM;               // East impact location in meters
  float fCoreSouthM;              // South impact location in meters
  float fCoreElevationMASL;       // Height of impact location in meters ASL
  // ***************************************************
  VSimulationData(){};
  VSimulationData(uword32 fCORSIKAParticleID,
		  float fEnergyGeV, float fObservationZenithDeg,
		  float fObservationAzimuthDeg, float fPrimaryZenithDeg,
		  float fPrimaryAzimuthDeg, float fRefZenithDeg, 
		  float fRefAzimuthDeg, float fRefPositionAngleDeg,
		  float fCoreEastM, float fCoreSouthM, 
		  float fCoreElevationMASL):
    fCORSIKAParticleID(fCORSIKAParticleID),
    fEnergyGeV(fEnergyGeV),
    fObservationZenithDeg(fObservationZenithDeg),
    fObservationAzimuthDeg(fObservationAzimuthDeg),
    fPrimaryZenithDeg(fPrimaryZenithDeg),
    fPrimaryAzimuthDeg(fPrimaryAzimuthDeg),
    fRefZenithDeg(fRefZenithDeg),
    fRefAzimuthDeg(fRefAzimuthDeg),
    fRefPositionAngleDeg(fRefPositionAngleDeg),
    fCoreEastM(fCoreEastM),
    fCoreSouthM(fCoreSouthM),
    fCoreElevationMASL(fCoreElevationMASL){};
  
  virtual ~VSimulationData() {};
    
  // bank operations - you probably don't want to use these directly.
  uword32 getBankVersion();
  uword32 getBankSize();
  void writeBankToBuffer(char *buf);
  uword32 calcBankAdler(uword32 adler);
  void writeBankToFile(word64 offset,
                         int fd);
    
  // lower-level writing operation
  void writeTo(FPOutputBase *out);

  // Copy method.
  VSimulationData* copySimData();
};

const VBankName &VGetSimulationDataBankName();
VBankBuilder *VGetSimulationDataBankBuilder();

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
