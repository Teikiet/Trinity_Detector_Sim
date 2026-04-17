/*
 * VSimulationHeader.h -- a bank that stores the header simulation data
 * by Glenn Sembroski, 2004
 *
 * This bank is based on Fillip Pizlo's original VSimulationData
 */

#ifndef V_SIMULATION_HEADER_H
#define V_SIMULATION_HEADER_H

#define V_SIMULATION_HEADER_VERSION 0

#include "VBank.h"
#include "VBankBuilder.h"

#include "FPIO.h"
#include "VException.h"

#include <string>
#include <vector>
#include "Adler32.h"

struct VPixelLocation
{    
  float fPixLocEastAtStowDeg;
  float fPixLocUpAtStowDeg;
  float fPixRadiusDeg;
};


struct VArrayConfiguration
{            // the telescope locations
  float fRelTelLocSouthM; // position relative to array center
  float fRelTelLocEastM;  // position relative to array center
  float fRelTelLocUpM;    // elevation relative to obs level

  std::vector<VPixelLocation> fCamera;
};
#ifndef SIMULATION_CODES
#define SIMULATION_CODES
enum SimulationPackageCodes   {KNOWNNOT,LEEDS,GRISU,KASCADE,CORSIKA,UCLA};
//enum SimulatorCodes           {SOMEBODYELSE,SEMBROSKI,ISU,GERNOT,VVV};
#endif

// ***********************************************************
// make it a struct so that everything is public.  that really makes most
// sense since otherwise it would just end up having a bunch of getters and
// setters.
struct VSimulationHeader:public VBank
{
  // ************************************************************************
  // This class is the base class for the simulation header bank. For a 
  // specific simulation (Ex. Kascade, GrISU etc), it is inherited by
  // that specific simulation's simulation header bank class.
  // ************************************************************************
  // FP: I tacked this  field onto here; I figured it  won't hurt!
  // (note: if you're creating this structure in order to save it in a
  // VBF file, you DO NOT have to set this field; in fact, even if you
  // do set it, it will be ignored!  however, this field will be
  // automatically set for you when you read a VSimulationHeader bank from
  // a VBF file.)
  long fRunNumber;
  
  // *************************************************
  //Below is what gets written to simulation header bank
  // *************************************************
  uword32 fDateOfSimsUTC;        //in the format YYYYMMDD, SJF
  uword32 fSimulationPackage;   // Codes to be determined. Suggestions below.
  //uword32 fSimulator;
  std::string fSimulator;
  uword32 fDateOfArrayForSims;  // specifies the epoch for the  array 
                                //that the simulations are to mimic
                                //in the format YYYYMMDD, SJF
  uword32 fAtmosphericModel;    //Corsika nomenclature
  float   fObsAltitudeM;        // [metres above sea level]
  std::vector<VArrayConfiguration> fArray;
  std::string  fSimConfigFile;  // ASCII text desribing the simulation  
                                // operating parameters

  // ***************************************************
  VSimulationHeader(){};
  //  VSimulationHeader(uword32 fDateOfSimsUTC, uword32 fSimulationPackage,
  //		    uword32 fSimulator, uword32 fDateOfArrayForSims,
  //		    uword32 fAtmosphericModel, float fObsAltitudeM,
  //		    std::vector<VArrayConfiguration> fArray,
  //		    std::string  fSimConfigFile):
  VSimulationHeader(uword32 fDateOfSimsUTC, uword32 fSimulationPackage,
		    std::string fSimulator, uword32 fDateOfArrayForSims,
		    uword32 fAtmosphericModel, float fObsAltitudeM,
		    std::vector<VArrayConfiguration> fArray,
		    std::string  fSimConfigFile):
    fDateOfSimsUTC(fDateOfSimsUTC),
    fSimulationPackage(fSimulationPackage),
    fSimulator(fSimulator),
    fDateOfArrayForSims(fDateOfArrayForSims),
    fAtmosphericModel(fAtmosphericModel),
    fObsAltitudeM(fObsAltitudeM),
    fArray(fArray),
    fSimConfigFile(fSimConfigFile){};
  virtual ~VSimulationHeader() {};
    
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

const VBankName &VGetSimulationHeaderBankName();
VBankBuilder *VGetSimulationHeaderBankBuilder();

/*
-------------------------------------------------------------------------------
CODES USED (TO BE EXTENDED AS NEEDED)
     -------------------------------------------------------------------------------

----- fSimulationPackage -----

Basically just informational.. describes what package was used in creation of
the file. These are just for example . actual settings may vary . see enum's
above.

0) Unknown
1) Leeds        - CORSIKA/GrISUdet
2) Grinnel/ISU  - Kascade/GrISUdet
3) Purdue       - Kascade/ksAomega
4) UCLA/Chicago - CORSIKA/ChiLA
5) ...


*/

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
