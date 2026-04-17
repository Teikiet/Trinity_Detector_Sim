
#ifndef VBITPARSER_H
#define VBITPARSER_H

#include <vector>
#include <cmath>

#include "VRawDataExceptions.h"

#include <stdint.h> 
 
typedef uint32_t index_t;

class VBitParser {
 public:
  VBitParser()  noexcept ;
  ~VBitParser()  noexcept {};
 void setAddress(uint32_t *ptr, size_t nBits=512)  noexcept ;
 //  void setBuffer(spVBuffer_t sptr, Index_t start,  size_t nBits=512)
 //    throw(VIndexOutOfBoundsException);
  
  // Non-Modifying Operations:
  
  size_t getNumBitsHit()  noexcept ; 
  index_t getHitBit(index_t i)  ;
  std::vector<index_t> getHitVec(); 
  std::vector<bool> getFullHitVec() { if (fModified) update(); return fFullHitVec;}; 
  bool checkBit(index_t);  
  
  // Modifying Operations:
  
  void setBit(index_t, bool)  ;
  void resetAllBits();
  //  void write10(Index_t dest, uint16_t bits)  ;
  
 private:

  void update()  noexcept ; // should be private?
  
  uint32_t &element(index_t i) {return *(fpBits+i);}
  uint32_t element(index_t i) const {return *(fpBits+i);}


  uint32_t * fpBits;

  //  spVBuffer_t fspBuffer;
  //  Index_t fStartIndex;

  index_t fBit;
  size_t fBitMax;
  bool fModified;

  size_t fNumHit;
  std::vector<index_t> fHitVec;
  std::vector<bool> fFullHitVec;
    
};

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
