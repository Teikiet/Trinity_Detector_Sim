#include "VGPSDecoder.h"

void VGPSDecoder::decode(uint32_t word0, uint32_t word1, uint32_t word2)  noexcept {

  // hopefully code below is endian independant!

  uint16_t TimeArray[5];
   TimeArray[0]=word0 & 0x0000FFFF;
   TimeArray[1]=(word0 & 0xFFFF0000) >> 16;
   TimeArray[2]=word1 & 0x0000FFFF;
   TimeArray[3]=(word1 & 0xFFFF0000) >> 16;
   TimeArray[4]=word2 & 0x0000FFFF;


  
//   uint32_t timeVal[3];
//   timeVal[0]=word0;
//   timeVal[1]=word1;
//   timeVal[2]=word2;
//   uint16_t *TimeArray = (uint16_t*)timeVal;

  // what follows as copied from Scott:

  //fGPSStatus = (TimeArray[1]>>4 & 0xF);  //This was changed 20/03/07 By GHS 
   fGPSStatus = (TimeArray[0]>>4 & 0xF);   //as directed by JQ

  fGPSDays = 100*(TimeArray[0] & 0x000F)   
    + 10*(TimeArray[1]>>12 & 0x000F) 
    + (TimeArray[1]>>8 & 0x000F);
 
  fGPSHrs  = 10* (TimeArray[1]>>4 & 0x000F)   + (TimeArray[1]>>0 & 0x000F);
  fGPSMins = 10* (TimeArray[2]>>12 & 0x000F)  + (TimeArray[2]>>8 & 0x000F);
  fGPSSecs = 10* (TimeArray[2]>>4 & 0x000F)   + (TimeArray[2]>>0 & 0x000F);

  fGPSSecs += 1E-1*(TimeArray[3]>>12 & 0x000F) +
    1E-2*(TimeArray[3]>>8 & 0x000F)  +
    1E-3*(TimeArray[3]>>4 & 0x000F)  +
    1E-4*(TimeArray[3]>>0 & 0x000F)  +
    1E-5*(TimeArray[4]>>12 & 0x000F) +
    1E-6*(TimeArray[4]>>8 & 0x000F)  +
    1E-7*(TimeArray[4]>>4 & 0x000F);

  return;
}


void VGPSDecoder::printDataToStream(std::ostream& stream)  noexcept { 
  stream<<"Status:          "<<(int) getStatus()<<std::endl;
  stream<<"Days:            "<<(int) getDays()<<std::endl;
  stream<<"Hours:           "<<(int) getHrs()<<std::endl;
  stream<<"Minutes:         "<<(int) getMins()<<std::endl;
  stream<<"Seconds:         "<<(double) getSecs()<<std::endl;
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
