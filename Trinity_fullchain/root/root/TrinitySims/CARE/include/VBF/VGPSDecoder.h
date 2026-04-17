#ifndef VGPSDECODER_H
#define VGPSDECODER_H
  
#include <stdint.h>
#include <iostream>

#include "VRawEventParser.h"
#include "VDatum.h"

class VGPSDecoder
{
 public:
  VGPSDecoder() {};
  ~VGPSDecoder() {};
  
  void decode(uint32_t word0, uint32_t word1, uint32_t word2)  noexcept ;

  void decode(uint16_t word0,
	      uint16_t word1,
	      uint16_t word2,
	      uint16_t word3,
	      uint16_t word4) {
    decode(word0|(word1<<16),
	   word2|(word3<<16),
	   word4);
  }

  void decode(VRawEventParser *parser) {
    decode(parser->getGPS0(),
	   parser->getGPS1(),
	   parser->getGPS2());
  }

  void decode(VEvent *event) {
      decode(event->getGPSTime()[0],
	     event->getGPSTime()[1],
	     event->getGPSTime()[2],
	     event->getGPSTime()[3],
	     event->getGPSTime()[4]);
  }
  
  int getStatus()  noexcept {return fGPSStatus;}
  int getDays()  noexcept {return fGPSDays;} 
  int getHrs()  noexcept {return fGPSHrs;}
  int getMins()  noexcept {return fGPSMins;}
  double getSecs()  noexcept {return fGPSSecs;}
      
  void printDataToStream(std::ostream& stream)  noexcept ;

 private:

  int fGPSStatus;
  int fGPSDays;
  int fGPSHrs;
  int fGPSMins;
  double fGPSSecs;
  
};




inline std::ostream& operator<<(std::ostream &stream, VGPSDecoder &decoder) 
{ 
    decoder.printDataToStream(stream); 
    return(stream); 
} 


#endif // VGPSDECODER_H
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
