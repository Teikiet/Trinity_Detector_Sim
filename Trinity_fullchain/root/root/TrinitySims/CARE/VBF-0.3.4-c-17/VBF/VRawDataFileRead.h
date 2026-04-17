#ifndef VRAWDATAFILEREAD_H
#define VRAWDATAFILEREAD_H

#include <exception>
#include <string>

#include <stdint.h> // for uint32_t

// for open(), stat()
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "VRawEventData.h"
#include "VRawDataExceptions.h"
#include "VBankFileReader.h"

//////////////////////////////////////////////////////////////////////////////
//////// THIS IS LEGACY CODE!  PLEASE SWITCH TO VBankFileReader!!!!! /////////
//////////////////////////////////////////////////////////////////////////////
  
class VRawDataFileRead
{
 public:
  VRawDataFileRead(std::string fileName,
                   int telescopeId=0);
  
  ~VRawDataFileRead()
    {
      if (fFD>=0) {
        ::close(fFD);
      }
      if (fReader!=NULL) {
        delete fReader;
      }
    }

  // true if successful, false EOF, exception if error
  bool getNextEvent(VRawEventData &);

  // this is a special method for reading raw telescope data.  IT DOES NOT
  // WORK for Harvester and VBF files.  the purpose of this method is to
  // read data where the events are larger than the limit imposed by the
  // 16-bit size in the header.
  bool getNextLargeTelEvent(VRawEventData &)  ;
  
  // tells you if you're using a VBF file
  bool isBankFile() const noexcept {
    return fFileType == VBF;
  }
  
  // ONLY WORKS IF USING A VBF FILE!
  // returns true if there was an event at that index, false if there
  // wasn't.  note that there may be 'holes' in a file where there are
  // no events, so a false return does not mean EOF.
  bool getEventByIndex(unsigned index,
                       VRawEventData &);
  
  // ONLY WORKS IF USING A VBF FILE!
  // returns an exclusive upper bound on the index of the last event.
  // to put it another way, there are no events to be found for indices
  // greater than or equal to the value returned by this method.
  unsigned getEventIndexUpperBound()  ;

  bool rewindFile();      //SPW
  size_t getEventCount(); //SPW  


 private:
  inline uint32_t swapEndian(uint32_t in);
  inline uint32_t readLE32(int fd, uint32_t * buf, size_t nwords);
  
  enum eStatus {Read_Event, Rejected_Event, End_Of_File};
  eStatus getNextTelescopeEvent(VRawEventData &,
                                uint32_t &totalSize);
  eStatus getNextHarvesterEvent(VRawEventData &);
  eStatus getNextVBFEvent(VRawEventData &);

  int fFD;
  enum eEndian {Little_Endian, Big_Endian};
  eEndian fEndian;
  
  enum eFileType {Undetermined, Telescope, Harvester, VBF};
  eFileType fFileType;
  
  int fTelescopeId;
  
  unsigned fIndex;
  VBankFileReader *fReader;
};


inline uint32_t VRawDataFileRead::swapEndian(uint32_t in)
{
  uint32_t dum;
  dum  = (in & 0xFF000000) >>24;
  dum |= (in & 0x00FF0000) >> 8;
  dum |= (in & 0x0000FF00) << 8;
  dum |= (in & 0x000000FF) << 24;
  return dum;
}


inline uint32_t VRawDataFileRead::readLE32(int fd, 
                                           uint32_t * buf, 
                                           size_t nwords)
{
  uint32_t nbytesread = read (fd, buf, nwords*4);
  uint32_t n32read= nbytesread/4;

  if (fEndian == Big_Endian){
    for (uint32_t i = 0; i < n32read;i++)
      *(buf+i) = swapEndian(*(buf+i));
  }

  return n32read;
}


#endif //VRAWDATAFILEREAD_H
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
