#ifndef VRAWEVENTPARSER_H
#define VRAWEVENTPARSER_H

#include <cmath>    // for ceil()

#include <vector>

#include "VRawDataExceptions.h"
#include "VBitParser.h"
#include "VRawEventData.h"
#include "VDatum.h"

//#include "VCTParser.h"



class VRawEventParser
{
 public:
  VRawEventParser()  noexcept : fVectorsFilled(false) {};
  ~VRawEventParser()  noexcept {};

  void setEvent(VRawEventData *)  ; 
  void setEvent(VEvent *)  noexcept ; 

  uint32_t getSync() {return fSync;}  // 'VEVN'
  uint8_t  getNode() {return fNode;}   
  uint8_t  getTelTrigMask() {return fTelTrigMask;}  
  uint8_t  getEventType() {return fEventType;}  
  uint16_t getDataLength() { return fDataLength;}
  uint32_t getEventNumber() { return fEventNumber;}

  // quick and dirty temporary way to get at GPS data
  uint32_t getGPSWord(unsigned index) {
    if (fEv!=NULL) {
	  if (index<2) {
	      return ((fEv->getGPSTime()[index*2+1]<<16)|
		      fEv->getGPSTime()[index*2]);
	  } else {
	      return fEv->getGPSTime()[4];
	  }
    } else {
      return fpData[fGPSStartIndex+index];
    }
  }
  uint32_t getGPS0() {return getGPSWord(0);}
  uint32_t getGPS1() {return getGPSWord(1);}
  uint32_t getGPS2() {return getGPSWord(2);}

  uint16_t getGPSYear() {return fYear;}
  uint32_t getStatus() {return fStatus;}
  uint16_t getNumSamples() {return fNumSamples;}
  uint16_t getNumChannelsHit() {return fNumChannelsHit;} // i.e. with data
  uint16_t getMaxChannels() {return fMaxChannels;}
  uint16_t getNumCT() {return fNumCT;}

  // related to hit (i.e. which channels have data) pattern
  size_t getNumBitsHit() { return fHitBitParser.getNumBitsHit();}

  // id of the ith hit channel
  uint32_t getHitID(uint32_t i) {return fHitBitParser.getHitBit(i);} 
  
  // vector of hit channel ids
  std::vector<uint32_t> getHitVec() { return fHitBitParser.getHitVec();}

  // all channels - 1 of hit, 0 if not. useful for plotting
  std::vector<bool> getFullHitVec() {return fHitBitParser.getFullHitVec();}

  // CFD trigger pattern - 1 if hit, 0 otherwise
  std::vector<bool> getFullTrigVec() {return fTrigBitParser.getFullHitVec();}

  // CT raw data
  std::vector<uint32_t> getCT(uint32_t)  ;

  // check if a given channel was hit and get its index
  std::pair<bool, uint32_t> getChannelHitIndex(uint32_t);


  //------------------------------------------------------------------------
  // channel related data - must select which hit channel you want data for 

  // select the ith hit channel
  void selectHitChan(uint32_t)  ;

  //get sample j for the selected channel
  uint8_t getSample(uint32_t)  ;

  // get a vector of all samples for selected channel
  std::vector<uint8_t> getSamplesVec() {return fSelectedChanSamplesVec;}


  // get pulse area (from FADC area header) 
  uint16_t getCharge()
    {
      if (fEv!=NULL) {
        return fEv->getCharge(fSelectedHitChan);
      } else {
        return ((element(fSelectedChanIndex + fSamplesWordSize) & 0xFFFF0000) >>16) ;
      }
    }


  // get area under pulse (simple sum of samples)
  uint32_t getSummedSamplesCharge()
    {
      return fSelectedChanSummedSamplesCharge;
    }


  uint16_t getPedestal() 
    {
      if (fEv!=NULL) {
        return fEv->getPedestalAndHiLo(fSelectedHitChan)&32767;
      } else {
        return (element(fSelectedChanIndex + fSamplesWordSize) & 0x00007FFF) ;
      }
    }

  uint16_t getHiLo() 
    {
      if (fEv!=NULL) {
        return fEv->getPedestalAndHiLo(fSelectedHitChan)>>15;
      } else {
        return (element(fSelectedChanIndex + fSamplesWordSize) & 0x00008000)>>15 ;
      }
    }




  //------------------------------------------------------------------------
  // all channels at once - useful for plotting on camera widget

  // get a vector of pulse areas (from summing FADC samples) for all channels
  std::vector<uint32_t> getSummedSamplesChargeVec() 
    {
      if (!fVectorsFilled)
        fillVectors();

      return fSummedSamplesChargeVec;
    }

  // get a vector of pulse areas (from FADC area header) for all channels
  std::vector<uint32_t> getFullChargeVec() {
    if (!fVectorsFilled)
      fillVectors();
    
    return fFullChargeVec;
  }

  void fillVectors();

  //------------------------------------------------------------------------

  
  
 private:

  uint32_t element(uint32_t i) {return *(fpData+i);}
  
  VEvent*                fEv;
  
  uint32_t*              fpData;
  size_t                 fSize;

  std::vector<uint32_t>  fFullChargeVec;
  std::vector<uint32_t>  fSummedSamplesChargeVec;
  size_t                 fSelectedChanSummedSamplesCharge;

  uint32_t               fSelectedHitChan;
  uint32_t               fSelectedChanIndex;
  std::vector<uint8_t>   fSelectedChanSamplesVec;
  uint32_t               fSamplesWordSize;
  uint32_t               fChanDataStartIndex;
  size_t                 fBitPatternWords;

  uint32_t               fSync;
  uint8_t                fNode;
  uint8_t                fTelTrigMask;
  uint8_t                fEventType;
  uint16_t               fDataLength;
  uint32_t               fEventNumber;
  uint32_t               fGPSStartIndex;
  uint16_t               fYear;
  uint32_t               fStatus;
  uint16_t               fNumSamples;
  uint16_t               fNumChannelsHit;
  uint16_t               fMaxChannels;
  uint16_t               fNumCT;
  uint32_t               fFirstCTOffset;
  

  VBitParser             fTrigBitParser;
  VBitParser             fHitBitParser;

  uint16_t               fCurrChanPed;
  uint16_t               fCurrChanCharge;

  //  VCTParser             fCTParser; // make public?


  static const uint32_t fgkHeaderSize;
  static const uint32_t fgkExpectedSync;
  static const uint32_t fgkGPSSize;
  static const uint32_t fgkCTSize;
 
  bool fVectorsFilled;

};


#endif //VRAWEVENTPARSER_H
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
