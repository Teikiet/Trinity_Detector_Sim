/*
 * VSimulationHeader.cpp -- implementation of simulation header data class
 * by Glenn Sembroski
 */

#include "VSimulationHeader.h"

#include <iostream>
#include "FPBasics.h"
#include "FPAdler.h"
#include "FPFDWrite.h"
#include "FPFDRead.h"

static VBank *buildBankFrom(long run_number,
                            uword32 event_number,
			    uword32 version,
                            FPInputBase *in) {
    if (version>V_SIMULATION_HEADER_VERSION) {
	throw VBankBuilderVersionException("While building VSimulationHeader");
    }

                                // operating parameters
    uword32 fDateOfSimsUTC;
    uword32 fSimulationPackage;
    uword32 fLength;
    std::string fSimulator;
    //uword32 fSimulator;
    uword32 fDateOfArrayForSims;
    uword32 fAtmosphericModel;
    float   fObsAltitudeM;
    std::vector<VArrayConfiguration> fArray;
    std::string fSimConfigFile;
    
    fDateOfSimsUTC      = in->readWord32();
    fSimulationPackage  = in->readWord32();
    fLength             = in->readWord32();
    char *fSimulatorBuf = new char[fLength];
                          in->readBytes(fSimulatorBuf,fLength);
    fSimulator          = std::string(fSimulatorBuf,fLength);
    delete []fSimulatorBuf;
    //fSimulator          = in->readWord32();
    fDateOfArrayForSims = in->readWord32();
    fAtmosphericModel   = in->readWord32();
    fObsAltitudeM       = in->readFloat();

    // ******************************************************************
    // We are going to read the array configuration into a vector
    // Get number of various elements from file
    // ******************************************************************
    uword32 fNumTelsInArrayConfig;
    uword32 fNumPixelsInCamera;

    fNumTelsInArrayConfig = in->readUWord32();
    fArray.resize(fNumTelsInArrayConfig);
    for(uword32 i=0;i<fNumTelsInArrayConfig;i++)
      {
	fArray[i].fRelTelLocSouthM = in->readFloat();
	fArray[i].fRelTelLocEastM  = in->readFloat();
	fArray[i].fRelTelLocUpM    = in->readFloat();

	fNumPixelsInCamera         = in->readWord32();
	fArray[i].fCamera.resize(fNumPixelsInCamera);
	for(uword32 j=0;j<fNumPixelsInCamera;j++)
	  {
	    fArray[i].fCamera[j].fPixLocEastAtStowDeg = in->readFloat();
	    fArray[i].fCamera[j].fPixLocUpAtStowDeg   = in->readFloat();
	    fArray[i].fCamera[j].fPixRadiusDeg        = in->readFloat();
	  }
      }

    // *********************************************************************
    // Read in configuration file string
    // *********************************************************************
    fLength = in->readUWord32();
    char *fBuf=new char[fLength];
    in->readBytes(fBuf,fLength);
    fSimConfigFile = std::string(fBuf,fLength);  //because not null terminated
    delete []fBuf;
    VSimulationHeader *result=
        new VSimulationHeader(fDateOfSimsUTC, fSimulationPackage,fSimulator,
			      fDateOfArrayForSims, fAtmosphericModel, 
			      fObsAltitudeM,fArray,fSimConfigFile);
      
    result->fRunNumber=run_number;
    return result;
}

class VSimulationHeaderBankBuilder:
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

static VBankName bank_name("HeadSimu"); //Generic simulation bank name is 
                                        //CoreSimu
static VSimulationHeaderBankBuilder bank_builder;

const VBankName &VGetSimulationHeaderBankName() {
    return bank_name;
}

VBankBuilder *VGetSimulationHeaderBankBuilder() {
    return &bank_builder;
}

uword32 VSimulationHeader::getBankVersion() {
    return 0;
}

uword32 VSimulationHeader::getBankSize() 
{
  uword32 fBankSizeBytes = 4 //fDateOfSimsUTC
                          +4; //fSimulationPackage
  int fLength=fSimulator.size();
  fBankSizeBytes += 4;       //fLength
  fBankSizeBytes += fLength; //fSimConfigFile
  fBankSizeBytes +=        4 //fDateOfArrayForSims
                          +4 //fAtmosphericModel
                          +4;//fObsAltitudeM
  int fNumTels=fArray.size();
  fBankSizeBytes += 4;       //fNumTels
  for(int i=0;i<fNumTels;i++)
    {
      fBankSizeBytes += 4 //fRelTelLocSouthM
	               +4 //fRelTelLocEastM
	               +4;//fRelTelLocUpM
      int fNumPixels=fArray[i].fCamera.size();
      fBankSizeBytes += 4;       //fNumPixels
      for(int j=0;j<fNumPixels;j++)
	{
	  fBankSizeBytes += 4 //fPixLocEastAtStowDeg
                           +4 //fPixLocUpAtStowDeg
	                   +4;//fPixRadiusDeg
	}
    }

  fLength=fSimConfigFile.size();
  fBankSizeBytes += 4;       //fLength
  fBankSizeBytes += fLength; //fSimConfigFile
  return fBankSizeBytes;
}

void VSimulationHeader::writeTo(FPOutputBase *out) {
  out->writeWord(fDateOfSimsUTC);
  out->writeWord(fSimulationPackage); 

  out->writeWord((uword32)fSimulator.size());
  out->writeBytes(fSimulator.data(),fSimulator.size());
  //  out->writeWord(fSimulator);

  out->writeWord(fDateOfArrayForSims); 
  out->writeWord(fAtmosphericModel);
  out->writeWord(fObsAltitudeM);
  int fNumTels=fArray.size();
  out->writeWord((uword32)fNumTels);
  for(int i=0;i<fNumTels;i++)
    {
      out->writeWord(fArray[i].fRelTelLocSouthM);
      out->writeWord(fArray[i].fRelTelLocEastM);
      out->writeWord(fArray[i].fRelTelLocUpM);
      int fNumPixels=fArray[i].fCamera.size();
      out->writeWord((uword32)fNumPixels);
      for(int j=0;j<fNumPixels;j++)
	{
	  out->writeWord(fArray[i].fCamera[j].fPixLocEastAtStowDeg);  
	  out->writeWord(fArray[i].fCamera[j].fPixLocUpAtStowDeg);  
	  out->writeWord(fArray[i].fCamera[j].fPixRadiusDeg);  
	}
    }
  out->writeWord((uword32)fSimConfigFile.size());
  out->writeBytes(fSimConfigFile.data(),fSimConfigFile.size());
}


void VSimulationHeader::writeBankToBuffer(char *buf) {
    FPBasicOut out(buf,getBankSize(),false);
    writeTo(&out);
}

uword32 VSimulationHeader::calcBankAdler(uword32 adler) {
    FPAdlerOut out(adler);
    writeTo(&out);
    return out.getAdler();
}

void VSimulationHeader::writeBankToFile(word64 offset,
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
