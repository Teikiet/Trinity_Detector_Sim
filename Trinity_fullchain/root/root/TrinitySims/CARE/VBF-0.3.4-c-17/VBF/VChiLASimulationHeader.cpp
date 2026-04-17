/*
 * VChiLASimulationHeader.cpp -- implementation of ChiLA simulation header
 * class
 * by Matthew Wood and Stephen Fegan
 */

#include "VChiLASimulationHeader.h"

#include "FPBasics.h"
#include "FPAdler.h"
#include "FPFDWrite.h"
#include "FPFDRead.h"

static VBank *buildBankFrom(long run_number,
                            uword32 event_number,
			    uword32 version,
                            FPInputBase *in) 
{
  if (version>V_CHILA_SIMULATION_HEADER_VERSION) 
    {
      throw VBankBuilderVersionException("While building "
					 "VChiLASimulationHeader");
    }
 // *********************************************************************
  // ChiLA specific Simulation bank data
  // *********************************************************************
  uword32 fDatabaseNameLength;
  std::string fDatabaseName;
  uword32 fNumSimParamTables;
  std::vector<VChiLASimParamTable> fSimParamTables;
  uword32 fNumOpticsConfigurations;
  std::vector<VChiLAOpticsConfiguration> fOpticsConfigurations;
  uword32 fNumElectronicsConfigurations;
  std::vector<VChiLAElectronicsConfiguration> fElectronicsConfigurations;

  fDatabaseNameLength = in->readWord32();
  char *fDatabaseNameBuf = new char[fDatabaseNameLength];
  in->readBytes(fDatabaseNameBuf,fDatabaseNameLength);
  fDatabaseName          = std::string(fDatabaseNameBuf,fDatabaseNameLength);
  delete []fDatabaseNameBuf;

  fNumSimParamTables = in->readWord32();
  fSimParamTables.resize(fNumSimParamTables);
  for(uword32 i = 0; i < fNumSimParamTables; i++) 
    {
      fSimParamTables[i].fTableID   = in->readWord32();
      fSimParamTables[i].fPrimaryID = in->readWord32();
      fSimParamTables[i].fEnergyTeV = in->readFloat();
      fSimParamTables[i].fZenithMinRad = in->readFloat();
      fSimParamTables[i].fZenithMaxRad = in->readFloat();
      fSimParamTables[i].fAzimuthMinRad = in->readFloat();
      fSimParamTables[i].fAzimuthMaxRad = in->readFloat();
      fSimParamTables[i].fOpticsID  = in->readWord32();
      fSimParamTables[i].fSamplingRadiusM = in->readFloat();
      fSimParamTables[i].fEventCount = in->readWord32();
      uword32 fTableNameLength = in->readWord32();
      char *fTableNameBuf = new char[fTableNameLength];
      in->readBytes(fTableNameBuf,fTableNameLength);
      fSimParamTables[i].fTableName = 
	std::string(fTableNameBuf,fTableNameLength);
      delete []fTableNameBuf;
    }

  fNumOpticsConfigurations = in->readWord32();
  fOpticsConfigurations.resize(fNumOpticsConfigurations);
  for(uword32 i = 0; i < fNumOpticsConfigurations; i++) 
    {

      fOpticsConfigurations[i].fOpticsID = in->readWord32();
      uword32 fOpticsDumpLength = in->readWord32();
      char *fOpticsDumpBuf = new char[fOpticsDumpLength];
      in->readBytes(fOpticsDumpBuf,fOpticsDumpLength);
      fOpticsConfigurations[i].fOpticsDump = 
	std::string(fOpticsDumpBuf,fOpticsDumpLength);
      delete []fOpticsDumpBuf;
    }

  fNumElectronicsConfigurations = in->readWord32();
  fElectronicsConfigurations.resize(fNumElectronicsConfigurations);
  for(uword32 i = 0; i < fNumElectronicsConfigurations; i++) 
    {

      fElectronicsConfigurations[i].fElectronicsID = in->readWord32();
      uword32 fElectronicsDumpLength = in->readWord32();
      char *fElectronicsDumpBuf = new char[fElectronicsDumpLength];
      in->readBytes(fElectronicsDumpBuf,fElectronicsDumpLength);
      fElectronicsConfigurations[i].fElectronicsDump = 
	std::string(fElectronicsDumpBuf,fElectronicsDumpLength);
      delete []fElectronicsDumpBuf;

    }

  VChiLASimulationHeader *result=
    new VChiLASimulationHeader(fDatabaseName,
			       fSimParamTables,
			       fOpticsConfigurations,
			       fElectronicsConfigurations);
  result->fRunNumber=run_number;
  return result;
}

class VChiLASimulationHeaderBankBuilder:
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

static VBankName bank_name("ChLAHead");
static VChiLASimulationHeaderBankBuilder bank_builder;

const VBankName &VGetChiLASimulationHeaderBankName() {
    return bank_name;
}

VBankBuilder *VGetChiLASimulationHeaderBankBuilder() {
    return &bank_builder;
}

uword32 VChiLASimulationHeader::getBankVersion() {
    return 0;
}

uword32 VChiLASimulationHeader::getBankSize() 
{
  uword32 fSize = 0;
  uword32 fDatabaseNameLength=fDatabaseName.size();
  fSize += 4;                   // fDatabaseNameLength
  fSize += fDatabaseNameLength; // fDatabaseName

  uword32 fNumSimParamTables = fSimParamTables.size();
  fSize += 4;                   // fNumSimParamTables
  for(uword32 i = 0; i < fNumSimParamTables; i++)
    {
      fSize+=4 // fTableID
	+ 4    // fPrimaryID
	+ 4    // fEnergyTeV
	+ 4    // fZenithMinRad
	+ 4    // fZenithMaxRad
	+ 4    // fAzimuthMinRad
	+ 4    // fAzimuthMaxRad
	+ 4    // fOpticsID
	+ 4    // fSamplingRadiusM
	+ 4;   // fEventCount
      uword32 fTableNameLength = 
	fSimParamTables[i].fTableName.size();
      fSize+=4; // fTableNameLength
      fSize+=fTableNameLength; // fTableName

    }

  uword32 fNumOpticsConfigurations=fOpticsConfigurations.size();
  fSize += 4;                   // fNumOpticsConfigurations
  for(uword32 i = 0; i < fNumOpticsConfigurations; i++)
    {
      fSize += 4; // fOpticsID
      uword32 fOpticsDumpLength=
	fOpticsConfigurations[i].fOpticsDump.size();
      fSize += 4; // fOpticsDumpLength
      fSize += fOpticsDumpLength; // fOpticsDump

    }

  uword32 fNumElectronicsConfigurations=fElectronicsConfigurations.size();
  fSize += 4;                   // fNumElectronicsConfigurations
  for(uword32 i = 0; i < fNumElectronicsConfigurations; i++)
    {
      fSize += 4; // fElectronicsID
      uword32 fElectronicsDumpLength=
	fElectronicsConfigurations[i].fElectronicsDump.size();
      fSize += 4; // fElectronicsDumpLength
      fSize += fElectronicsDumpLength; // fElectronicsDump

    }


 return fSize; 
}

void VChiLASimulationHeader::writeTo(FPOutputBase *out) 
{

  out->writeWord((uword32)fDatabaseName.size());
  out->writeBytes(fDatabaseName.data(),fDatabaseName.size());

  uword32 fNumSimParamTables=fSimParamTables.size();
  out->writeWord((uword32)fNumSimParamTables);
  for(uword32 i=0;i<fNumSimParamTables;i++)
    {
      
      out->writeWord(fSimParamTables[i].fTableID);
      out->writeWord(fSimParamTables[i].fPrimaryID);
      out->writeWord(fSimParamTables[i].fEnergyTeV);
      out->writeWord(fSimParamTables[i].fZenithMinRad);
      out->writeWord(fSimParamTables[i].fZenithMaxRad);
      out->writeWord(fSimParamTables[i].fAzimuthMinRad);
      out->writeWord(fSimParamTables[i].fAzimuthMaxRad);
      out->writeWord(fSimParamTables[i].fOpticsID);
      out->writeWord(fSimParamTables[i].fSamplingRadiusM);
      out->writeWord(fSimParamTables[i].fEventCount);
      out->writeWord((uword32)fSimParamTables[i].fTableName.size());
      out->writeBytes(fSimParamTables[i].fTableName.data(),
		      fSimParamTables[i].fTableName.size());

    }

  uword32 fNumOpticsConfigurations=fOpticsConfigurations.size();
  out->writeWord((uword32)fNumOpticsConfigurations);
  for(uword32 i=0;i<fNumOpticsConfigurations;i++)
    {
      out->writeWord(fOpticsConfigurations[i].fOpticsID);
      out->writeWord((uword32)fOpticsConfigurations[i].fOpticsDump.size());
      out->writeBytes(fOpticsConfigurations[i].fOpticsDump.data(),
		      fOpticsConfigurations[i].fOpticsDump.size());
    }

  uword32 fNumElectronicsConfigurations=fElectronicsConfigurations.size();
  out->writeWord((uword32)fNumElectronicsConfigurations);
  for(uword32 i=0;i<fNumElectronicsConfigurations;i++)
    {
      out->writeWord(fElectronicsConfigurations[i].fElectronicsID);
      out->writeWord((uword32)fElectronicsConfigurations[i].fElectronicsDump.size());
      out->writeBytes(fElectronicsConfigurations[i].fElectronicsDump.data(),
		      fElectronicsConfigurations[i].fElectronicsDump.size());
    }


}

void VChiLASimulationHeader::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    writeTo(&out);
}

uword32 VChiLASimulationHeader::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    writeTo(&out);
    return out.getAdler();
}

void VChiLASimulationHeader::writeBankToFile(word64 offset,
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
