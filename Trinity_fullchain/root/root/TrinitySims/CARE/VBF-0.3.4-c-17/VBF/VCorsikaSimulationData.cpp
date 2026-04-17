//-*-mode:c++; mode:font-lock;-*-
/*
 * VCorsikaSimulationData.cpp -- implementation of corsika simulation data classes
 * by Henrike Fleischhack
 */

#include "VCorsikaSimulationData.h"

#include "FPBasics.h"
#include "FPAdler.h"
#include "FPFDWrite.h"
#include "FPFDRead.h"

static VBank *buildBankFrom(long run_number,
                            uword32 event_number,
			    uword32 version,
                            FPInputBase *in) {
    if (version>V_CORSIKA_SIMULATION_DATA_VERSION) 
      {	
	throw VBankBuilderVersionException("Bad version requested while "
                                           "building VCorsikaSimulationData");
      }
    // *********************************************************************
    // Corsika specific Simulation bank data
    // *********************************************************************
    float fFirstInteractionDepth;
    float fFirstInteractionHeight;
    word32 fShowerID;
    word32 fCorsikaRunID;


    if(version==0)
      {
        fFirstInteractionHeight     = in->readFloat();
        fFirstInteractionDepth      = in->readFloat();
        fShowerID                   = in->readWord32();
        fCorsikaRunID               = in->readWord32();
      }
    else
      {
	throw VBankBuilderVersionException("Verison err "
                                               " VCorsikaSimulationData");
      }

        
    VCorsikaSimulationData *result=new VCorsikaSimulationData( fFirstInteractionHeight, fFirstInteractionDepth, fCorsikaRunID, fShowerID  );
    result->fRunNumber=run_number;
    result->fEventNumber=event_number;
    
    return result;
}

class VCorsikaSimulationDataBankBuilder:
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

static VBankName bank_name("CorsSim");
static VCorsikaSimulationDataBankBuilder bank_builder;

const VBankName &VGetCorsikaSimulationDataBankName() {
    return bank_name;
}

VBankBuilder *VGetCorsikaSimulationDataBankBuilder() {
    return &bank_builder;
}

uword32 VCorsikaSimulationData::getBankVersion() 
{
   return 0;
}

uword32 VCorsikaSimulationData::getBankSize() 
{
  uword32 fSize=4 //fFirstInteractionHeight
    +4//fFirstInteractionDepth          
    +4//fShowerID                
    +4;//fCorsikaRunID            
  return fSize; 
}

void VCorsikaSimulationData::writeTo(FPOutputBase *out) 
{
  out->writeWord(fFirstInteractionHeight);
  out->writeWord(fFirstInteractionDepth);
  out->writeWord(fShowerID);
  out->writeWord(fCorsikaRunID);
}

void VCorsikaSimulationData::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    writeTo(&out);
}

uword32 VCorsikaSimulationData::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    writeTo(&out);
    return out.getAdler();
}

void VCorsikaSimulationData::writeBankToFile(word64 offset,
                                      int fd) {
    FPFDWrite out(fd,offset);
    writeTo(&out);
}
// ***************************************************************************

VCorsikaSimulationData* VCorsikaSimulationData::copyCorsikaSimData()
// ********************************************************
// Make a new VCorsikaSimulationData and fill it with the data from this one.
// ********************************************************
{
  VCorsikaSimulationData* pfNewKSim= new VCorsikaSimulationData( fFirstInteractionHeight, fFirstInteractionDepth, fCorsikaRunID, fShowerID);
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
