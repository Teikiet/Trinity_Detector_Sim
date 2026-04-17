#include "VRawEventParser.h"

#include <algorithm>
#include <iostream>

const uint32_t VRawEventParser::fgkHeaderSize = 9; // 32-bit words
const uint32_t VRawEventParser::fgkExpectedSync = 0x4e564556; 
const uint32_t VRawEventParser::fgkGPSSize = 3;
const uint32_t VRawEventParser::fgkCTSize = 7;


std::pair<bool, uint32_t> VRawEventParser::getChannelHitIndex(uint32_t chan)
{
  std::vector<uint32_t> hitVec= fHitBitParser.getHitVec();
  std::vector<uint32_t>::iterator hitIndexIter;    // hit index of channel if relevant
  
  hitIndexIter=find(hitVec.begin(), hitVec.end(), chan);
  
  if(hitIndexIter == hitVec.end()) // i.e. not found
    return std::make_pair(false, (uint32_t) 0);
  else
    return std::make_pair(true, (uint32_t) (hitIndexIter - hitVec.begin()) );
}


      
std::vector<uint32_t> VRawEventParser::getCT(uint32_t whichCT)
{
  if( whichCT > fNumCT) {
    throw VIndexOutOfBoundsException("VEventParser::getCT()",
                                     whichCT,
                                     0,
                                     fNumCT);
  }

  std::vector<uint32_t> tmpCTVec;
  tmpCTVec.reserve(fgkCTSize);
  
  if (fEv!=NULL) {
    for (unsigned i=0;i<fgkCTSize; ++i) {
	tmpCTVec.push_back(fEv->getClockTrigData(whichCT)[i]);
    }
  } else {
    uint32_t startIndex = fFirstCTOffset + whichCT * fgkCTSize;
    
    for(uint32_t i=0;i<fgkCTSize; i++)
      tmpCTVec.push_back(element(startIndex + i));
  }
  
  return tmpCTVec;
}



void VRawEventParser::setEvent(VRawEventData *pEvent)  {
  fVectorsFilled=false;
  
  if (pEvent->fpData==NULL) {
    setEvent(pEvent->fEv);
    return;
  }

  fEv = NULL;
  
  fpData = pEvent->fpData;
  fSize = pEvent->fSize;
  
  fSync = element(0);

  //  std::cout<<"@ "<<index<<": 0x"<<std::hex<<element(index)<<std::dec<<std::endl;


  if (fSync != fgkExpectedSync){
    throw VSyncException("VEventParser::setEvent()",
                         fgkExpectedSync,
                         fSync);
  }

  if( fSize < fgkHeaderSize) {
    throw VIndexOutOfBoundsException("VEventParser::setEvent()",
                                     fgkHeaderSize,
                                     0,
                                     fSize);
  }

  // ok - buffer is big enough for headers - so parse

  // fSync alread done;

  uint32_t dummy32;

  uint32_t index=0;

  index++;

  //  node, trig mask and event size
  dummy32 = element(index);
  fNode       = dummy32 & 0x000000FF;
  fTelTrigMask   = (dummy32 & 0x0000FF00)>>8;
  fDataLength = (dummy32 & 0xFFFF0000)>>16;
  index ++;

  // Event Number  
  fEventNumber = element(index);
  index++;

  // GPS
  fGPSStartIndex=index;
  index+=fgkGPSSize -1; // -1 since the event type is bits 8-16 (counting 
  // from 0) of last (3rd) GPS word;
  
  fYear = ((element(index)  & 0x0000FF00)>>8);
  fEventType = (element(index) & 0x000000FF);
  index++;

  // Status
  fStatus = element(index);
  index++;

  // Num Samples and Num Channels;
  dummy32 = element(index);
  fNumSamples     = dummy32 & 0x0000FFFF;
  fNumChannelsHit = (dummy32 & 0xFFFF0000) >> 16;
  fSamplesWordSize=fNumSamples/4;
  
  fSelectedChanSamplesVec.assign(fNumSamples,0);

  index++;

  // max chan and number of CT boards
  dummy32 = element(index);
  fMaxChannels = dummy32 & 0x0000FFFF;
  fNumCT       = (dummy32 & 0xFFFF0000) >> 16;
  index++;
  
  fBitPatternWords = (uint32_t) std::ceil(fMaxChannels/32.0);


  // now calculate size of rest of data
  size_t variableDataSize =  2 * fBitPatternWords   // bit Patterns;
    + fNumChannelsHit *(fSamplesWordSize + 1) // 1 for charge/ped
    + fNumCT * fgkCTSize;


  fChanDataStartIndex = fgkHeaderSize + 2 * fBitPatternWords;


  if( fSize < (fgkHeaderSize + variableDataSize)){
    VIndexOutOfBoundsException ex("VEventParser::setEvent()",
                                  fgkHeaderSize + variableDataSize,
                                  0,
                                  fSize);
    ex.stream()<<" - Buffer not big enough for variable length data." <<std::endl;
    throw ex;
  }


  fHitBitParser.setAddress(fpData  +fgkHeaderSize, 
                          fMaxChannels);


  fTrigBitParser.setAddress(fpData + fgkHeaderSize+fBitPatternWords, 
                           fMaxChannels);

  
  fFirstCTOffset = fgkHeaderSize + variableDataSize - fNumCT * fgkCTSize;
    
}

void VRawEventParser::setEvent(VEvent *pEvent)  noexcept {
  fVectorsFilled=false;
  fEv = pEvent;
  fSize = pEvent->getSize();
  
  // the VEvent would have already done sanity checks, hence we don't have
  // to do them here.
  
  //  node, trig mask and event size
  fNode = fEv->getNodeNumber();
  fTelTrigMask = fEv->getTriggerMask();
  fDataLength = fEv->getSize()-8; // FIXME: this is bogus.

  // Event Number  
  fEventNumber = fEv->getEventNumber();

  fEventType = fEv->getRawEventTypeCode();
  fYear = fEv->getGPSYear();

  // Status
  fStatus = fEv->getFlags();

  // Num Samples and Num Channels;
  fNumSamples     = fEv->getNumSamples();
  fNumChannelsHit = fEv->getNumChannels();
  
  fSelectedChanSamplesVec.assign(fNumSamples,0);

  // max chan and number of CT boards
  fMaxChannels = fEv->getMaxNumChannels();
  fNumCT       = fEv->getNumClockTrigBoards();
  
  fBitPatternWords = (uint32_t) std::ceil(fMaxChannels/32.0);


  fHitBitParser.setAddress(fEv->getHitPattern(), 
                           fMaxChannels);


  fTrigBitParser.setAddress(fEv->getTriggerPattern(), 
                            fMaxChannels);
}


void VRawEventParser::fillVectors()
{
  // vector will be initialised with all zeros
  fFullChargeVec = std::vector<uint32_t>(fMaxChannels,0);
  fSummedSamplesChargeVec = std::vector<uint32_t>(fMaxChannels,0);

   for(uint32_t i=0;i<getNumBitsHit();i++){
     uint16_t chanID = getHitID(i);
     selectHitChan(i);
     uint32_t charge = getCharge(); // + multiply by 10 * hilo??
     fFullChargeVec[chanID]=charge;
    
     std::vector<uint8_t> tmpSamplesVec = getSamplesVec();
     for (uint32_t j=0;j<tmpSamplesVec.size(); j++){
       fSummedSamplesChargeVec[chanID] += tmpSamplesVec[j];
     }
   }
}

 
void VRawEventParser::selectHitChan(uint32_t hit)
{
  //  fSelectedChanSamplesVec.clear();

  if( hit > fNumChannelsHit){
    throw VIndexOutOfBoundsException("VEventParser::selectHitChan()",
                                     hit,
                                     0,
                                     fNumChannelsHit);
  }
  
  fSelectedHitChan = hit;
  
  if (fEv==NULL) {
    fSelectedChanIndex= fChanDataStartIndex + hit*(fSamplesWordSize +1);
  }
  
  fSelectedChanSummedSamplesCharge=0;

  for(uint32_t i=0;i<fNumSamples;i++){
    uint8_t i_Sample=getSample(i);

    fSelectedChanSamplesVec[i]=i_Sample;
    fSelectedChanSummedSamplesCharge+=i_Sample;
  }
}


 
 

uint8_t VRawEventParser::getSample(uint32_t sample)  {
  if( sample >= fNumSamples){
    throw VIndexOutOfBoundsException("VEventParser::getSample()",
                                     sample,
                                     0,
                                     fNumSamples-1);
  }
  
  if (fEv!=NULL) {
    return fEv->getSample(fSelectedHitChan, sample);
  }
  
  uint32_t offset= (uint32_t) sample/4;
  uint32_t word = element(fSelectedChanIndex+offset);

  switch((int) sample-(offset*4)){
  case 0:
    return(word & 0x000000FF);
    break;
  case 1:
    return((word & 0x0000FF00)>>8);
    break;
  case 2:
    return((word & 0x00FF0000)>>16);
    break;
  case 3:
    return((word & 0xFF000000)>>24);
    break;
  } // end switch

  return(0);

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
