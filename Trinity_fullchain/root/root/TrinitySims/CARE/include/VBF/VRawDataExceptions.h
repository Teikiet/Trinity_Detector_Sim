#ifndef VRAWDATAEXCEPTIONS_H
#define VRAWDATAEXCEPTIONS_H

#include "VException.h"

#include <stdint.h>

//---------------------------------------------------------------------------
// VFileException
//---------------------------------------------------------------------------


class VFileException : public VException
{
 public: 
  VFileException(const std::string what="VFileException", 
                 const std::string where="")
    :VException(what, where) {}; 

  virtual ~VFileException()  noexcept {};

};


//---------------------------------------------------------------------------
// VFileOpenException
//---------------------------------------------------------------------------


class VFileOpenException : public VFileException
{
 public: 
  VFileOpenException(const std::string& where)
    :VFileException("VFileOpenException", where) {}; 

  ~VFileOpenException()  noexcept {};

};


//---------------------------------------------------------------------------
// VFileReadException
//---------------------------------------------------------------------------


class VFileReadException : public VFileException
{
 public: 
  VFileReadException(const std::string& where)
    :VFileException("VFileReadException", where) {}; 

  ~VFileReadException()  noexcept {};

};


//---------------------------------------------------------------------------
// VSyncException
//---------------------------------------------------------------------------

class VSyncException : public VException
{
 public:
  VSyncException(const std::string& where, 
                 uint32_t expected,
                 uint32_t encountered)
    : VException("VSyncException", where), 
      fExpected(expected), fEncountered(encountered) {}

  // no special copy constructors needed

  virtual ~VSyncException()  noexcept { /* nothing to see here */ }

  uint32_t getExpected() const {return fExpected;}
  uint32_t getEncountered() const {return fEncountered;}

  virtual void printTypeSpecificExceptionDetails(std::ostream& stream) const;

 private:
  uint32_t fExpected;      // sync pattern encountered expected
  uint32_t fEncountered;   // actual sync pattern encountered

};





//---------------------------------------------------------------------------
// VIndexOutOfBoundsException
//---------------------------------------------------------------------------

class VIndexOutOfBoundsException : public VException
{
 public:
  VIndexOutOfBoundsException(const std::string& where, 
                             uint32_t index,
                             uint32_t min,
                             uint32_t max)
    : VException("VIndexOutOfBoundsException", where), 
      fIndex(index), fMin(min), fMax(max) {}

  // no special copy constructors needed

  virtual ~VIndexOutOfBoundsException()  noexcept { /* nothing to see here */ }

  uint32_t getIndex() const {return fIndex;}
  uint32_t getMin() const {return fMin;}
  uint32_t getMax() const {return fMax;}

  virtual void printTypeSpecificExceptionDetails(std::ostream& stream) const
    {
      stream << " - Index " << getIndex() <<" not in allowed range " 
             << getMin() << " to " << getMax() <<std::endl;
    }


 private:
  uint32_t fIndex; // index which casued exception
  uint32_t fMin;   // minimum allowable index
  uint32_t fMax;   // maximum allowable index
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
