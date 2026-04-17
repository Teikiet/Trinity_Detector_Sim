#include "VRawDataFileRead.h"

#define V_EVENT_MAGIC 0x4e564556  // VEVN
#define V_ARRAY_EVENT_MAGIC 0x56454156  // VAEV
#define V_RUN_FILE_MAGIC 0x46525256 // VRRF
#define V_BANK_FILE_MAGIC 0x46464256 // VBFF

VRawDataFileRead::VRawDataFileRead(std::string fileName,
                                   int telescopeId):
  fReader(NULL)
{

  // first determine endianness of the system:
  union {
    uint32_t word;
    uint8_t bytes[4];
  } u;
  
  u.word=1;
  if (u.bytes[0]==1){
    // little endian!
    fEndian=Little_Endian;
  }
  else if (u.bytes[3]==1){
    // big endian!
    fEndian=Big_Endian;
  }
  else{
   ; // error - do something!
   V_ASSERT(false);
  }

  //-----------------------------------

  fFD=open(fileName.c_str(),O_RDONLY);
  
  if(fFD<0){
    VFileOpenException ex("VRawDataFileRead::VRawDataFileRead()");
    ex.stream()<<" - error opening file "<<fileName<<std::endl;
    
    throw(ex);
  }
  
  try {
    uint32_t sync;
    uint32_t n32read = readLE32 (fFD, &sync, 1);
    
    if (n32read<0) {
      VFileReadException ex("VRawDataFileRead::VRawDataFileRead()");
      
      ex.stream()<<" - Experienced system error while reading file header."<<std::endl;
      ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
      throw(ex);
    } else if (n32read==0) {
      // eof.  we set the type to Undetermined.  user will
      // catch EOF in getNextEvent()
      fFileType=Undetermined;
    } else {
      switch (sync) {
        case V_EVENT_MAGIC:
          fFileType=Telescope;
          
          // getNextTelescopeEvent() expects to
          // see the magic numbers, so we back up
          // to prevent confusion.
          if (lseek(fFD,0,SEEK_SET)<0) {
            VFileReadException ex("VRawDataFileRead::VRawDataFileRead()");

            ex.stream()<<" - Experienced system error while seeking to beginning of file."<<std::endl;
            ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
          }
          break;
        case V_RUN_FILE_MAGIC:
          fFileType=Harvester;
          break;
        case V_BANK_FILE_MAGIC:
          ::close(fFD);
          fFD=-1;
          try {
            fReader=new VBankFileReader(fileName,
                                        true,  // using an index
                                        true); // read-only
          } catch (const std::exception &e) {
            VFileReadException ex("VRawDataFileRead::VRawDataFileRead()");
            ex.stream()<<" - Could not open a VBF file."<<std::endl;
            ex.stream()<<" - The VBF error was: "<<e.what()<<std::endl;
            throw ex;
          }
          
          fIndex=0;
          fFileType=VBF;
          break;
        default:
          {
            VFileReadException ex("VRawDataFileRead::VRawDataFileRead()");

            ex.stream()<<" - Could not find either 'VEVN' or 'VRRF' at "
                         "beggining of file."<<std::endl;
            ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
            throw(ex);
          }
      }
    }
  } catch (...) {
    if (fFD>=0) {
      ::close(fFD);
    }
    throw;
  }
  
  fTelescopeId=telescopeId;
}


bool VRawDataFileRead::getNextEvent(VRawEventData &event)  {
  for (;;) {
    eStatus s;
    uint32_t _; // ML notation meaning 'unused value'
    switch (fFileType) {
      case Undetermined:
        return false;
      case Telescope:
        s=getNextTelescopeEvent(event,_);
        break;
      case Harvester:
        s=getNextHarvesterEvent(event);
        break;
      case VBF:
        s=getNextVBFEvent(event);
        break;
      default:
        V_ASSERT(false);
    }
    switch (s) {
      case Read_Event:
        return true;
      case End_Of_File:
        return false;
      default:
        break;
    }
  }
}

VRawDataFileRead::eStatus VRawDataFileRead::getNextVBFEvent(VRawEventData &event) {
  if (fIndex >= getEventIndexUpperBound()) {
    return End_Of_File;
  }
  
  return getEventByIndex(fIndex++,event)?Read_Event
                                        :Rejected_Event;
}

bool VRawDataFileRead::getEventByIndex(unsigned index,
                                       VRawEventData &event)  {
  if (!isBankFile()) {
    VFileReadException ex("VRawDataFileRead::getEventByIndex()");
    ex.stream()<<" - Attempt to do random access in a file that does not"<<std::endl;
    ex.stream()<<"   support it."<<std::endl;
    throw ex;
  }
  
  try {
    if (!fReader->hasPacket(index)) {
      return false;
    }
    
    VPacket *packet=fReader->readPacket(index);
    
    if (!packet->hasArrayEvent()) {
      delete packet;
      return false;
    }
    
    VEvent *ev=packet->getArrayEvent()->getEventByNodeNumber(fTelescopeId);
    
    if (ev==NULL) {
      delete packet;
      return false;
    }
    
    event.reset();
    event.fPacket=packet;
    event.fEv=ev;
    
    return true;
  } catch (const std::exception &e) {
    VFileReadException ex("VRawDataFileRead::getEventByIndex()");
    ex.stream()<<" - Experienced VBF error while reading event."<<std::endl;
    ex.stream()<<" - VBF error was: "<<e.what()<<std::endl;
    throw ex;
  }
}

unsigned VRawDataFileRead::getEventIndexUpperBound()  {
  if (!isBankFile()) {
    VFileReadException ex("VRawDataFileRead::getMaxEventIndex()");
    ex.stream()<<" - Attempt to do random access in a file that does not"<<std::endl;
    ex.stream()<<"   support it."<<std::endl;
    throw ex;
  }
  
  return fReader->numPackets();
}

VRawDataFileRead::eStatus VRawDataFileRead::getNextHarvesterEvent(VRawEventData &event) {
  uint32_t len;
  
  uint32_t res=readLE32(fFD,&len,1);
  if (res==0) {
    return End_Of_File;
  } else if (res<0) {
    VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

    ex.stream()<<" - Experienced system error while reading array event length."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }
  
  if (len<9) {
    VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

    ex.stream()<<" - Array event size is smaller than 9 bytes, which is"<<std::endl;
    ex.stream()<<"   insufficient to hold the array event header."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  }
  
  uint32_t sync;
  res=readLE32(fFD,&sync,1);
  if (res==0) {
    VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

    ex.stream()<<" - Read array event size but god EOF reading array event"<<std::endl;
    ex.stream()<<"   magic number."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  } else if (res<0) {
    VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

    ex.stream()<<" - Experienced system error while reading array event sync."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }
  
  if (sync != V_ARRAY_EVENT_MAGIC) {
    VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

    ex.stream()<<" - Sync 'VAEV' expected but not encountered."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  }
  
  uint8_t numDatums;
  res=read(fFD,&numDatums,1);
  if (res==0) {
    VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

    ex.stream()<<" - Read array event size and sync but got EOF reading datum"<<std::endl;
    ex.stream()<<"   count byte."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  } else if (res<0) {
    VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

    ex.stream()<<" - Experienced system error while reading datum count byte."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }
  
  // skip the run number
  if (lseek(fFD,4,SEEK_CUR)<0) {
    VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

    ex.stream()<<" - Experienced system error while skipping run number."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }
  
  // ok.  we got through the array event header.  now we read events until
  // we find the one we want.
  
  len-=9; // already read header.
  for (uint8_t i=0;
       i<numDatums;
       ++i) {
    uint32_t sizeRead;
    eStatus s=getNextTelescopeEvent(event,sizeRead);
    len-=sizeRead;
    switch (s) {
      case Read_Event:
        // skip remaining bytes
        if (lseek(fFD,len,SEEK_CUR)<0) {
          VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

          ex.stream()<<" - Experienced system error while skipping events."<<std::endl;
          ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
          throw(ex);
        }
        return Read_Event;
      case Rejected_Event:
        break;
      case End_Of_File:
        {
          VFileReadException ex("VRawDataFileRead::getNextHarvesterEvent()");

          ex.stream()<<" - Experienced EOF while reading an event even though the"<<std::endl;
          ex.stream()<<"   array event header claimed that this event would be there."<<std::endl;
          ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
          throw(ex);
        }
    }
  }
  
  return Rejected_Event;  // will get here if the array event did not contain
                          // the event we wanted.
}

VRawDataFileRead::eStatus
VRawDataFileRead::getNextTelescopeEvent(VRawEventData &event,
					uint32_t &totalSize) {
  uint32_t sync;
  uint32_t header;
  uint32_t telescopeId;
  uint32_t size; // in bytes


  //----------  read Sync header word
  uint32_t n32read = readLE32 (fFD, &sync, 1);

  if (n32read == 0) // eof
    return End_Of_File;
  else if (n32read<0) {
    VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");

    ex.stream()<<" - Experienced system error while reading event sync."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }  
  
  if (sync != V_EVENT_MAGIC){
    VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");

    ex.stream()<<" - Sync 'VEVN' expected but not encountered."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  }
  //-------------------------------------


  //----------  read event size
  n32read = readLE32 (fFD, &header, 1);

  if (n32read == 0){ // eof, but if here probably an error!
    VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");
    ex.stream()<<" - EOF or error while reading event header."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  } else if (n32read<0) {
    VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");

    ex.stream()<<" - Experienced system error while reading event header."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }
   
   telescopeId=(header&0xff);   
   size=((header&0xffff0000)>>16);
   
   totalSize=size+8;

   if ((int)telescopeId!=fTelescopeId) {
     if (lseek(fFD,size,SEEK_CUR)<0) {
       VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");

       ex.stream()<<" - Experienced system error while skipping an event."<<std::endl;
       ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
       throw(ex);
     }
     return Rejected_Event;
   }

   size_t nwordstotal = totalSize/4;
   // size in words =  (size in bytes of rest + 8 bytes alreay read)/4

   // Allocate space for data; 
   event.reset();
   event.fpData = new uint32_t[nwordstotal];
   event.fSize=nwordstotal;

   
   // rewind 8 bytes as we want to read complete VEvent
   if (lseek(fFD, -8, SEEK_CUR)<0) {
     VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");

     ex.stream()<<" - Experienced system error while seeking back to beginning"<<std::endl;
     ex.stream()<<"   of event."<<std::endl;
     ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
     throw(ex);
   }
   
   size_t nwordsread = readLE32(fFD, event.fpData, nwordstotal); 
   
   //cout<<"nwordsread: "<<nwordsread<<"  nwordstotal: "<<nwordstotal<<endl;
  
   if (nwordsread<0) {
    VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");
    ex.stream()<<" - Experienced system error while reading event."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
   } else if (nwordsread != nwordstotal){
    VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");
    ex.stream()<<" - EOF while reading event."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
   }

   return Read_Event;
}



  
  
bool VRawDataFileRead::getNextLargeTelEvent(VRawEventData &event)  {
  if (fFileType != Telescope) {
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");
    ex.stream()<<" - Cannot use getNextLargeTelEvent() on non-telescope file."<<std::endl;
    throw(ex);
  }  

  uint32_t sync;
  uint32_t header;
  uint32_t telescopeId;


  //----------  read Sync header word
  uint32_t n32read = readLE32 (fFD, &sync, 1);

  if (n32read == 0) // eof
    return End_Of_File;
  else if (n32read<0) {
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");

    ex.stream()<<" - Experienced system error while reading event sync."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }  
  
  if (sync != V_EVENT_MAGIC){
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");

    ex.stream()<<" - Sync 'VEVN' expected but not encountered."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  }
  //-------------------------------------


  n32read = readLE32 (fFD, &header, 1);

  if (n32read == 0){ // eof, but if here probably an error!
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");
    ex.stream()<<" - EOF or error while reading event header."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  } else if (n32read<0) {
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");

    ex.stream()<<" - Experienced system error while reading event header."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }
  
  telescopeId=(header&0xff);

  // read ahead to the num samples/num channels/max num channels/num clk trg fields
  if (lseek(fFD, 20, SEEK_CUR)<0) {
      VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");
      
      ex.stream()<<" - Experienced system error while skipping to num samples field"<<std::endl;
      ex.stream()<<"   of event."<<std::endl;
      ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
      throw(ex);
  }
   
  uint32_t stuff;
  n32read = readLE32 (fFD, &stuff, 1);

  if (n32read == 0){
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");
    ex.stream()<<" - EOF or error while reading num samples/num channels."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  } else if (n32read<0) {
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");

    ex.stream()<<" - Experienced system error while reading num samples/num channels."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }
  
  uint16_t numSamples=stuff&0xffff;
  uint16_t numChannels=(stuff>>16)&0xffff;

  n32read = readLE32 (fFD, &stuff, 1);

  if (n32read == 0){
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");
    ex.stream()<<" - EOF or error while reading max num channels/num clk trg boards."<<std::endl;
    ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
    throw(ex);
  } else if (n32read<0) {
    VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");

    ex.stream()<<" - Experienced system error while reading max num channels/num clk trk boards."<<std::endl;
    ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
    throw(ex);
  }
  
  uint16_t maxNumChannels=stuff&0xffff;
  uint16_t numClkTrgBrds=(stuff>>16)&0xffff;

  uint32_t totalSize=0+8+(20)+(8+(((maxNumChannels+31)>>5)*((4)))+(((maxNumChannels+31)>>5)*((4)))+((numChannels)*((((numSamples)*((1)))+4)))+((numClkTrgBrds)*((28))));
  
  // rewind as we want to read complete VEvent
  if (lseek(fFD, -8-20-8, SEEK_CUR)<0) {
      VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");
      
      ex.stream()<<" - Experienced system error while seeking back to beginning"<<std::endl;
      ex.stream()<<"   of event."<<std::endl;
      ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
      throw(ex);
  }
   
  if ((int)telescopeId!=fTelescopeId) {
      if (lseek(fFD,totalSize,SEEK_CUR)<0) {
	  VFileReadException ex("VRawDataFileRead::getNextLargeTelEvent()");
	  
	  ex.stream()<<" - Experienced system error while skipping an event."<<std::endl;
	  ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
	  throw(ex);
      }
      return Rejected_Event;
  }

  size_t nwordstotal = totalSize/4;
  // size in words =  (size in bytes of rest + 8 bytes alreay read)/4
  
  // Allocate space for data; 
  event.reset();
  event.fpData = new uint32_t[nwordstotal];
  event.fSize=nwordstotal;
  
  
  size_t nwordsread = readLE32(fFD, event.fpData, nwordstotal); 
  
  //cout<<"nwordsread: "<<nwordsread<<"  nwordstotal: "<<nwordstotal<<endl;
  
  if (nwordsread<0) {
      VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");
      ex.stream()<<" - Experienced system error while reading event."<<std::endl;
      ex.stream()<<" - System error was: "<<strerror(errno)<<std::endl;
      throw(ex);
  } else if (nwordsread != nwordstotal){
      VFileReadException ex("VRawDataFileRead::getNextTelescopeEvent()");
      ex.stream()<<" - EOF while reading event."<<std::endl;
      ex.stream()<<" - Data Possibly Corrupt."<<std::endl;
      throw(ex);
  }
  
  return Read_Event;
}



  
  
///********************************************************
/// SPW 
bool VRawDataFileRead::rewindFile()  {

  switch (fFileType) {
  case Undetermined:
    return false;
  case Telescope:
    lseek(fFD, 0, SEEK_SET);   // go to very start of file
    break;
  case Harvester:
    lseek(fFD, 4, SEEK_SET);   // go to start + array event sync pattern
    break;
  case VBF:
    fIndex=0;
    fReader->resetSequentialRead();
    break;
  default:
    V_ASSERT(false);
    }

  return true;
}

///********************************************************
/// SPW 
size_t VRawDataFileRead::getEventCount()  {
  VRawEventData    evt;
  size_t eventCount = 0;
  
  if (fFileType==VBF) {
    for (unsigned i=0;
         i<getEventIndexUpperBound();
         ++i) {
      if (getEventByIndex(i,evt)) {
        eventCount++;
      }
    }
  } else {
    off_t currentPosition = lseek(fFD, 0, SEEK_CUR);
    
    try{
      while(getNextEvent(evt)) eventCount++;
    } catch (...) {
      lseek(fFD, currentPosition, SEEK_SET);
      throw;
    }
    
    //Go back where we started
    lseek(fFD, currentPosition, SEEK_SET);
  }

  return eventCount;
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
