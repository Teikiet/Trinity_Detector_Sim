/*
 * VSimulationData.cpp -- implementation of simulation data class
 * by Filip Pizlo, 2004
 */

#include "VSimulationData.h"

#include "FPBasics.h"
#include "FPAdler.h"
#include "FPFDWrite.h"
#include "FPFDRead.h"

static VBank *buildBankFrom(long run_number,
                            uword32 event_number,
			    uword32 version,
                            FPInputBase *in) {
    if (version>V_SIMULATION_DATA_VERSION) {
	throw VBankBuilderVersionException("While building VSimulationData");
    }

    uword32 fCORSIKAParticleID;
    float fEnergyGeV; 
    float fObservationZenithDeg;
    float fObservationAzimuthDeg; 
    float fPrimaryZenithDeg;
    float fPrimaryAzimuthDeg; 
    float fRefZenithDeg;
    float fRefAzimuthDeg;   
    float fRefPositionAngleDeg;
    float fCoreEastM;
    float fCoreSouthM; 
    float fCoreElevationMASL;

    // ******************************************************************
    // In version 0 the Observation direction data did not exist.
    // The variables had different names: 
    // fEnergyGeV was e
    // fPrimaryZenithDeg was theta
    // fPrimaryAzimuthDeg was phi
    // fCoreEastM was r[1]
    // fCoreSouthM was r[2]
    // fCoreElevationMASL was r[3]
    // ******************************************************************
    if(version==0)
      {
    	fCORSIKAParticleID     = in->readWord32();
    	fEnergyGeV             = in->readFloat();
    	fPrimaryZenithDeg      = in->readFloat()*180.0/M_PI;
    	fPrimaryAzimuthDeg     = in->readFloat()*180.0/M_PI;
      	fObservationZenithDeg  = fPrimaryZenithDeg;
      	fObservationAzimuthDeg = fPrimaryAzimuthDeg;
	fRefZenithDeg          = fPrimaryZenithDeg;
	fRefAzimuthDeg         = fPrimaryAzimuthDeg;
	fRefPositionAngleDeg   = 0;
      	fCoreEastM             = in->readFloat();
      	fCoreSouthM            = in->readFloat();
      	fCoreElevationMASL     = in->readFloat();
      }
    else if(version==1)
      {
	fCORSIKAParticleID     = in->readWord32();
	fEnergyGeV             = in->readFloat();
	fObservationZenithDeg  = in->readFloat();
	fObservationAzimuthDeg = in->readFloat();
	fPrimaryZenithDeg      = in->readFloat();
	fPrimaryAzimuthDeg     = in->readFloat();
	fRefZenithDeg          = fPrimaryZenithDeg;
	fRefAzimuthDeg         = fPrimaryAzimuthDeg;
	fRefPositionAngleDeg   = 0;
	fCoreEastM             = in->readFloat();
	fCoreSouthM            = in->readFloat();
	fCoreElevationMASL     = in->readFloat();
      }
    else if(version==2)
      {
	fCORSIKAParticleID     = in->readWord32();
	fEnergyGeV             = in->readFloat();
	fObservationZenithDeg  = in->readFloat();
	fObservationAzimuthDeg = in->readFloat();
	fPrimaryZenithDeg      = in->readFloat();
	fPrimaryAzimuthDeg     = in->readFloat();
	fRefZenithDeg          = in->readFloat();
	fRefAzimuthDeg         = in->readFloat();
	fRefPositionAngleDeg   = in->readFloat();
	fCoreEastM             = in->readFloat();
	fCoreSouthM            = in->readFloat();
	fCoreElevationMASL     = in->readFloat();
      }
    else
      {
	throw VBankBuilderVersionException("Verison err VSimulationData");
      }

    VSimulationData *result=
        new VSimulationData(fCORSIKAParticleID,
		   fEnergyGeV, fObservationZenithDeg,
		   fObservationAzimuthDeg, fPrimaryZenithDeg,
		   fPrimaryAzimuthDeg, fRefZenithDeg, fRefAzimuthDeg, 
		   fRefPositionAngleDeg, fCoreEastM, fCoreSouthM, 
                   fCoreElevationMASL);
    
    result->fRunNumber=run_number;
    result->fEventNumber=event_number;
    
    return result;
}

class VSimulationDataBankBuilder:
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
            return buildBankFrom(run_number,
                                 event_number,
				 version,
                                 &in);
        }
        
        VBank *readBankFromFile(long run_number,
                                uword32 event_number,
                                const VBankName &name,
				uword32 version,
                                uword32 size,
                                off_t offset,
                                int fd) {
            FPFDRead in(fd,offset);
            return buildBankFrom(run_number,
                                 event_number,
				 version,
                                 &in);
        }
        
};

static VBankName bank_name("CoreSimu"); //Generic simulation bank name is 
                                        //CoreSimu
static VSimulationDataBankBuilder bank_builder;

const VBankName &VGetSimulationDataBankName() {
    return bank_name;
}

VBankBuilder *VGetSimulationDataBankBuilder() {
    return &bank_builder;
}

uword32 VSimulationData::getBankVersion() {
  //return 0;
  //return 1;
  return 2;
}

uword32 VSimulationData::getBankSize() 
{
  return 4 //  fCORSIKAParticleID 
    +4     // fEnergyGeV
    +4     // fObservationZenithDeg
    +4     // fObservationAzimuthDeg
    +4     // fPrimaryZenithDeg 
    +4     // fPrimaryAzimuthDeg
    +4     // fRefZenithDeg 
    +4     // fRefAzimuthDeg
    +4     // fRefPositionAngleDeg
    +4     // fCoreEastM 
    +4     // fCoreSouthM
    +4;    // fCoreElevationMASL   
}

void VSimulationData::writeTo(FPOutputBase *out) {
  out->writeWord(fCORSIKAParticleID);
  out->writeWord(fEnergyGeV); 
  out->writeWord(fObservationZenithDeg);
  out->writeWord(fObservationAzimuthDeg); 
  out->writeWord(fPrimaryZenithDeg);
  out->writeWord(fPrimaryAzimuthDeg); 
  out->writeWord(fRefZenithDeg);
  out->writeWord(fRefAzimuthDeg); 
  out->writeWord(fRefPositionAngleDeg); 
  out->writeWord(fCoreEastM);
  out->writeWord(fCoreSouthM); 
  out->writeWord(fCoreElevationMASL);
}

void VSimulationData::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    writeTo(&out);
}

uword32 VSimulationData::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    writeTo(&out);
    return out.getAdler();
}

void VSimulationData::writeBankToFile(word64 offset,
                                      int fd) {
    FPFDWrite out(fd,offset);
    writeTo(&out);
}
// ***********************************************************
 
VSimulationData* VSimulationData::copySimData()
// ********************************************************
// Make a new VSimulationData and fill it with the data from this one.
// ********************************************************
{
  VSimulationData* pfNewSim= 
    new VSimulationData(fCORSIKAParticleID,
			fEnergyGeV, fObservationZenithDeg,
			fObservationAzimuthDeg, fPrimaryZenithDeg, 
			fPrimaryAzimuthDeg, fRefZenithDeg, fRefAzimuthDeg,
			fRefPositionAngleDeg, fCoreEastM, fCoreSouthM, 
			fCoreElevationMASL);
  return pfNewSim;
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
