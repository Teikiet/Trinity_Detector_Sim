//-*-mode:c++; mode:font-lock;-*-
/*
 * VSimulationData.cpp -- implementation of kascade simulation data classes
 * by Glenn Sembroski
 */

#include "VKascadeSimulationData.h"

#include "FPBasics.h"
#include "FPAdler.h"
#include "FPFDWrite.h"
#include "FPFDRead.h"

static VBank *buildBankFrom(long run_number,
                            uword32 event_number,
			    uword32 version,
                            FPInputBase *in) {
    if (version>V_KASCADE_SIMULATION_DATA_VERSION) 
      {	
	throw VBankBuilderVersionException("Bad version requested while "
                                           "building VKascadeSimulationData");
      }
    // *********************************************************************
    // KASCADE specific Simulation bank data
    // *********************************************************************
    word32 fNx;
    word32 fNy;
    word32 fDirectionIndex;
    float  fEmissionAltitudeM; 
    float  fEmissionAltitudeSigma;
    float  fMuonRatio;
    float  fAomega;
    float  fRelTelTriggerTimeNS;
    float  fDifferentialRatePerEventHz;
    float  fIntegralRatePerEventHz;
    uword32 fShowerID;


    if(version==0)
      {
        fNx                         = in->readWord32();
        fNy                         = in->readWord32();
        fDirectionIndex             = in->readWord32();
        fEmissionAltitudeM          = in->readFloat();
        fEmissionAltitudeSigma      = in->readFloat();
        fMuonRatio                  = in->readFloat();
        fAomega                     = in->readFloat();
        fRelTelTriggerTimeNS        = in->readFloat();
        fDifferentialRatePerEventHz = in->readFloat();
        fIntegralRatePerEventHz     = in->readFloat();
        fShowerID                   = 0;
      }
    else if(version==1)   //Version 1 adds fSowerID:
      {
        fNx                         = in->readWord32();
        fNy                         = in->readWord32();
        fDirectionIndex             = in->readWord32();
        fEmissionAltitudeM          = in->readFloat();
        fEmissionAltitudeSigma      = in->readFloat();
        fMuonRatio                  = in->readFloat();
        fAomega                     = in->readFloat();
        fRelTelTriggerTimeNS        = in->readFloat();
        fDifferentialRatePerEventHz = in->readFloat();
        fIntegralRatePerEventHz     = in->readFloat();
        fShowerID                   = in->readWord32();
      }
    else
      {
	throw VBankBuilderVersionException("Verison err "
                                               " VKascadeSimulationData");
      }

        
     VKascadeSimulationData *result=new VKascadeSimulationData(fNx, fNy,
				   fDirectionIndex, fEmissionAltitudeM ,
				   fEmissionAltitudeSigma,fMuonRatio,fAomega,
				   fRelTelTriggerTimeNS,
				   fDifferentialRatePerEventHz,
				   fIntegralRatePerEventHz,fShowerID);
    result->fRunNumber=run_number;
    result->fEventNumber=event_number;
    
    return result;
}

class VKascadeSimulationDataBankBuilder:
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

static VBankName bank_name("KascdSim");
static VKascadeSimulationDataBankBuilder bank_builder;

const VBankName &VGetKascadeSimulationDataBankName() {
    return bank_name;
}

VBankBuilder *VGetKascadeSimulationDataBankBuilder() {
    return &bank_builder;
}

uword32 VKascadeSimulationData::getBankVersion() 
{
   //return 0;
   return 1;
}

uword32 VKascadeSimulationData::getBankSize() 
{
  uword32 fSize=4 //fNx        //Mount Grid coordinates.
    +4 //fNy
    +4 //fDirectionIndex       //Mount direction index
    +4 //fEmissionAltitudeM    //Altitude in meters of Max Chrenkov emission.
    +4 //fEmissionAltitudeSigma//Sigman of max Altitude in meters
    +4 //fMuonRatio            //Fractrion opf pe's due to muons in image.
    +4 //fAomega               //Area-solid angle contribution of this event
    +4 //fRelTelTriggerTimeNS
    +4 //fDifferentialRatePerEventHz
    +4 //fIntegralRatePerEventHz
    +4;//fShowerID             //Added version 1
  return fSize; 
}

void VKascadeSimulationData::writeTo(FPOutputBase *out) 
{
  out->writeWord(fNx); 
  out->writeWord(fNy); 
  out->writeWord(fDirectionIndex);
  out->writeWord(fEmissionAltitudeM); 
  out->writeWord(fEmissionAltitudeSigma);
  out->writeWord(fMuonRatio); 
  out->writeWord(fAomega);
  out->writeWord(fRelTelTriggerTimeNS);
  out->writeWord(fDifferentialRatePerEventHz);
  out->writeWord(fIntegralRatePerEventHz);
  out->writeWord(fShowerID);
}

void VKascadeSimulationData::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    writeTo(&out);
}

uword32 VKascadeSimulationData::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    writeTo(&out);
    return out.getAdler();
}

void VKascadeSimulationData::writeBankToFile(word64 offset,
                                      int fd) {
    FPFDWrite out(fd,offset);
    writeTo(&out);
}
// ***************************************************************************

VKascadeSimulationData* VKascadeSimulationData::copyKascadeSimData()
// ********************************************************
// Make a new VKascadeSimulationData and fill it with the data from this one.
// ********************************************************
{
  VKascadeSimulationData* pfNewKSim= new VKascadeSimulationData(fNx, fNy,
				   fDirectionIndex, fEmissionAltitudeM ,
				   fEmissionAltitudeSigma,fMuonRatio,fAomega,
				   fRelTelTriggerTimeNS,
				   fDifferentialRatePerEventHz,
				   fIntegralRatePerEventHz,fShowerID);
  return pfNewKSim;
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
