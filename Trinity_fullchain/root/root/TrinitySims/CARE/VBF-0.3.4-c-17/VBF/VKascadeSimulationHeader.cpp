/*
 * VKascadeSimulationHeader.cpp -- implementation of kascade simulation header
 * class
 * by Glenn Sembroski
 */

#include "VKascadeSimulationHeader.h"

#include "FPBasics.h"
#include "FPAdler.h"
#include "FPFDWrite.h"
#include "FPFDRead.h"

static VBank *buildBankFrom(long run_number,
                            uword32 event_number,
			    uword32 version,
                            FPInputBase *in) 
{
  if (version>0) 
    {
      throw VBankBuilderVersionException("While building "
					 "VKascadeSimulationHeader");
    }
 // *********************************************************************
  // KASCADE specific Simulation bank data
  // *********************************************************************
  uword32 fCORSIKAParticleID;
  float   fEnergyGeV;
  uword32 fShowerID;
  float   fXAreaWidthM;	// Grid width in x direction in meters.
  float   fYAreaWidthM;	// Grid width in y direction in merters.
  uword32 fNorthSouthGrid; // Flag for grid type(see enum in .h file)

  fCORSIKAParticleID = in->readWord32();
  fEnergyGeV         = in->readFloat();
  fShowerID          = in->readWord32();
  fXAreaWidthM       = in->readFloat();// Grid width in x direction in meters.
  fYAreaWidthM       = in->readFloat();// Grid width in y direction in merters.
  fNorthSouthGrid    = in->readWord32();// Flag for grid type(see enum)

  VKascadeSimulationHeader *result=
    new VKascadeSimulationHeader(fCORSIKAParticleID,fEnergyGeV, fShowerID,
				 fXAreaWidthM, fYAreaWidthM, fNorthSouthGrid);
  result->fRunNumber=run_number;
  return result;
}

class VKascadeSimulationHeaderBankBuilder:
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

static VBankName bank_name("KscdHead");
static VKascadeSimulationHeaderBankBuilder bank_builder;

const VBankName &VGetKascadeSimulationHeaderBankName() {
    return bank_name;
}

VBankBuilder *VGetKascadeSimulationHeaderBankBuilder() {
    return &bank_builder;
}

uword32 VKascadeSimulationHeader::getBankVersion() {
    return 0;
}

uword32 VKascadeSimulationHeader::getBankSize() 
{
  uword32 fSize= 4 //fCORSIKAParticleID;
    +4             //fEnergyGeV;
    +4             //fShowerID
    +4             //fXAreaWidthM
    +4             //fYAreaWidthM
    +4;            //fNorthSouthGrid
 return fSize; 
}

void VKascadeSimulationHeader::writeTo(FPOutputBase *out) 
{
  out->writeWord(fCORSIKAParticleID); 
  out->writeWord(fEnergyGeV); 
  out->writeWord(fShowerID);
  out->writeWord(fXAreaWidthM);
  out->writeWord(fYAreaWidthM);
  out->writeWord(fNorthSouthGrid);
}

void VKascadeSimulationHeader::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    writeTo(&out);
}

uword32 VKascadeSimulationHeader::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    writeTo(&out);
    return out.getAdler();
}

void VKascadeSimulationHeader::writeBankToFile(word64 offset,
                                      int fd) {
    FPFDWrite out(fd,offset);
    writeTo(&out);
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
