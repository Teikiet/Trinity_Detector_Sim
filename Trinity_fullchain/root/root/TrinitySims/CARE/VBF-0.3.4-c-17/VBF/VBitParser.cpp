#include "VBitParser.h"
#include <sstream>
#include <cmath> 

#include <iostream>
using namespace std;

// Note: automaticaly calculates minimum number of uints needed for 
// appropriate number of bits.t

VBitParser::VBitParser()  noexcept :fpBits(NULL),fBitMax(0), fModified(false), fNumHit(0)
{
}




bool VBitParser::checkBit(index_t bit) 
{
  if (bit >= fBitMax){
    throw VIndexOutOfBoundsException("VBitParser::checkBit()",
                                     bit,
                                     0,
                                     fBitMax);
  }


  index_t i=static_cast<index_t>((bit)/32); // i is which UInt bit is in
  index_t j=bit-i*32;                  // j is bit position in UInt i
  
  
  return (element(i) & (1<<j));

}


    



void VBitParser::setBit(index_t bit, bool state) 
{
  if (bit >= fBitMax){
    throw VIndexOutOfBoundsException("VBitParser::setBit()",
                                     bit,
                                     0,
                                     fBitMax);
  }


  index_t i=static_cast<index_t>((bit)/32); // i is which UInt bit is in
  index_t j=bit-i*32;                  // j is bit position in UInt i
  

  // to set a bit 'on' (e.g 3rd bit) must first reset to zero (AND with e.g. 11111011)
  // and then OR it with 1 (i.e. OR with 0000100)

  uint32_t mask = 0xFFFFFFFF ^ (1<<j);
  //  cout<<"** Setbit: mask: "<<hex<<mask<<endl;
  uint32_t invMask = ~mask;
  //  cout<<"** Setbit: invMask: "<<hex<<invMask<<endl;

  
  element(i)=element(i) & mask;

  if (state){
    element(i) = element(i) | invMask;
  }
    
  fModified=true;

  //  return (*(fpAddress+i))&(1<<j);
}



void VBitParser::resetAllBits()
{

  index_t numWords=static_cast<index_t>((fBitMax)/32); // total number of 32-bit words
  index_t numBits=fBitMax-numWords*32;             // bit position in last word
  
  
  for (index_t i=0;i<numWords;i++){
    element(i)=0;
  }


  //^ = XOR
  // This turns off the first numBits bits in the mask - now simply AND with
  // UInt32 to set those bits to zero. 

  uint32_t mask = 0xffffffff ^ (static_cast<uint32_t> 
                                (1<<numBits) -1 );
  
  
  element(numWords) = element(numWords) & mask;

  fModified=true;

}





void VBitParser::setAddress(uint32_t *ptr, size_t nBits)  noexcept {
  fpBits = ptr;
  fBitMax=nBits;
  fModified=true;
  fHitVec.reserve(nBits);
  fFullHitVec.reserve(nBits);
}


index_t VBitParser::getHitBit(index_t bit)  {
  if (fModified)
    update();

  if (bit > fNumHit){
    throw VIndexOutOfBoundsException("VBitParser::getHitBit()",
                                     bit,
                                     0,
                                     fNumHit);
  }

  return fHitVec[bit];

}


size_t VBitParser::getNumBitsHit()  noexcept {
  if (fModified)
    update();

  return fNumHit;
}


vector<index_t> VBitParser::getHitVec() 
{
  if (fModified) 
    update(); 

  return fHitVec;
}



void VBitParser::update()  noexcept {

  //  cout<<"VBitParser::update()"<<endl;
  fNumHit=0;
  fHitVec.clear();
  fFullHitVec.clear();

  for(index_t i=0;i<fBitMax;i++){
    bool hit = checkBit(i);
    fFullHitVec.push_back(hit);
    if (hit){
      fNumHit += 1;
      fHitVec.push_back(i);
    }
  }

  fModified=false;

  //  std::cout<<"**** VBitParser::update() vec size: "<<fFullHitVec.size()<<std::endl;

  return;
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
