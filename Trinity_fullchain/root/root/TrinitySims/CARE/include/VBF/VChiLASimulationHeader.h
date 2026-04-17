/*
 * VChiLASimulationHeader.h -- a class that stores specific ChiLA
 * simulation data in the VBF event
 * by Matthew Wood and Stephen Fegan
 *
 */

#ifndef V_CHILA_SIMULATION_HEADER_H
#define V_CHILA_SIMULATION_HEADER_H

#define V_CHILA_SIMULATION_HEADER_VERSION 0

#include "VBank.h"
#include "VBankBuilder.h"

#include "FPIO.h"
#include "VException.h"

#include <string>
#include <vector>
#include "Adler32.h"

struct VChiLAOpticsConfiguration
{
  uword32     fOpticsID;
  std::string fOpticsDump;

};

struct VChiLAElectronicsConfiguration
{
  uword32     fElectronicsID;
  std::string fElectronicsDump;

};

struct VChiLASimParamTable 
{
      uword32              fTableID;
      uword32              fPrimaryID;
      float                fEnergyTeV;
      float                fZenithMinRad;
      float                fZenithMaxRad;
      float                fAzimuthMinRad;
      float                fAzimuthMaxRad;
      uword32              fOpticsID;
      float                fSamplingRadiusM;
      uword32              fEventCount;
      std::string          fTableName;
};


// make it a struct so that everything is public.  that really makes most
// sense since otherwise it would just end up having a bunch of getters and
// setters.

// *********************************************************************
// Do not Inherit from VSimulationHeader. This is for the CHILA specific
// stuff only. Write(and read) standard Simulation banks and CHILASimulaiton
// banks sperately
// *********************************************************************

struct VChiLASimulationHeader:public VBank
{
  // ************************************************************************
  // FP: I tacked this  field onto here; I figured it  won't hurt!
  // (note: if you're creating this structure in order to save it in a
  // VBF file, you DO NOT have to set this field; in fact, even if you
  // do set it, it will be ignored!  however, this field will be
  // automatically set for you when you read a VChiLASimulationHeader bank 
  // from a VBF file.)
  long fRunNumber;
  // *********************************************************************
  // Below are the ChiLA specific parameters that go with each shower run.
  // *********************************************************************
  //Special stuff for ChiLA


  std::string fDatabaseName;
  std::vector<VChiLASimParamTable> fSimParamTables;
  std::vector<VChiLAOpticsConfiguration> fOpticsConfigurations;
  std::vector<VChiLAElectronicsConfiguration> fElectronicsConfigurations;

  VChiLASimulationHeader(){};
  VChiLASimulationHeader(std::string DatabaseName, 
			 std::vector<VChiLASimParamTable> SimParamTables,
			 std::vector<VChiLAOpticsConfiguration> OpticsConfigurations,
			 std::vector<VChiLAElectronicsConfiguration> ElectronicsConfigurations):
    fDatabaseName(DatabaseName),
       fSimParamTables(SimParamTables),
       fOpticsConfigurations(OpticsConfigurations),
       fElectronicsConfigurations(ElectronicsConfigurations)
  {
    // ***********************************************************************
    // ChiLA stuff
    // ***********************************************************************
  
    //    fDatabaseName = DatabaseName;
    //    fSimParamTables = SimParamTables
    //    fOpticsConfigurations = OpticsConfigurations;
    //    fElectronicsConfigurations = ElectronicsConfigurations;

  };

  virtual ~VChiLASimulationHeader() {};
    
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

const VBankName &VGetChiLASimulationHeaderBankName();
VBankBuilder *VGetChiLASimulationHeaderBankBuilder();

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
