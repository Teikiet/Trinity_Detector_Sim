/*
 * VChiLASimulationData.cpp -- implementation of ChiLA simulation data classes
 * by Matthew Wood and Stephen Fegan
 */

#include "VChiLASimulationData.h"

#include "FPBasics.h"
#include "FPAdler.h"
#include "FPFDWrite.h"
#include "FPFDRead.h"

static VBank *buildBankFrom(long run_number,
                            uword32 event_number,
			    uword32 version,
                            FPInputBase *in) {
    if (version>V_CHILA_SIMULATION_DATA_VERSION)
      {
	throw VBankBuilderVersionException("While building "
					   "VChiLASimulationData");
      }
    // *********************************************************************
    // ChiLA specific Simulation bank data
    // *********************************************************************
    uword32 fSimParamTableID = 0;
    uword32 fElectronicsID = 0;
    uword32 fTableIndex = 0;

    float   fAomega = 0;
    float   fAomegaVar = 0;

    if(version==0)
      {
	fSimParamTableID          = in->readWord32();
	fElectronicsID            = in->readWord32();
	fTableIndex               = 0;
	fAomega                   = 0;
	fAomegaVar                = 0;
      }
    else if(version == 1)
      {
	fSimParamTableID          = in->readWord32();
	fElectronicsID            = in->readWord32();
	fTableIndex               = in->readWord32();
	fAomega                   = in->readFloat();
	fAomegaVar                = in->readFloat();
      }

    VChiLASimulationData *result=
      new VChiLASimulationData(fSimParamTableID,
			       fElectronicsID,
			       fTableIndex,
			       fAomega,
			       fAomegaVar);
    
    result->fRunNumber=run_number;
    result->fEventNumber=event_number;
    
    return result;
}

class VChiLASimulationDataBankBuilder:
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

static VBankName bank_name("ChLASimu");
static VChiLASimulationDataBankBuilder bank_builder;

const VBankName &VGetChiLASimulationDataBankName() {
    return bank_name;
}

VBankBuilder *VGetChiLASimulationDataBankBuilder() {
    return &bank_builder;
}

uword32 VChiLASimulationData::getBankVersion() {
  //    return 0;
  return 1;
}

uword32 VChiLASimulationData::getBankSize() 
{
  uword32 fSize=4  // fSimParamTableID
    +4             // fElectronicsID
    +4             // fTableIndex
    +4             // fAomega
    +4;            // fAomegaVar

  return fSize; 
}

void VChiLASimulationData::writeTo(FPOutputBase *out) 
{
  out->writeWord(fSimParamTableID); 
  out->writeWord(fElectronicsID); 
  out->writeWord(fTableIndex); 
  out->writeWord(fAomega); 
  out->writeWord(fAomegaVar); 
}

void VChiLASimulationData::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    writeTo(&out);
}

uword32 VChiLASimulationData::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    writeTo(&out);
    return out.getAdler();
}

void VChiLASimulationData::writeBankToFile(word64 offset,
					   int fd) {
    FPFDWrite out(fd,offset);
    writeTo(&out);
}
// ***************************************************************************

VChiLASimulationData* VChiLASimulationData::copyChiLASimData()
// ********************************************************
// Make a new VChiLASimulationData and fill it with the data from this one.
// ********************************************************
{
  VChiLASimulationData* pfNewSim= new VChiLASimulationData(fSimParamTableID,
							   fElectronicsID,
							   fTableIndex,
							   fAomega, 
							   fAomegaVar);
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
