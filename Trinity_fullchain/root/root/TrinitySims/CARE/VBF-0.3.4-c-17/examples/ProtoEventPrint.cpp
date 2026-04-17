#include <iostream>
using namespace std;

#include <VBF/VRawDataFileRead.h>
#include <VBF/VRawDataExceptions.h>
#include <VBF/VRawEventData.h>
#include <VBF/VRawEventParser.h>
#include <VBF/VGPSDecoder.h>


int main(int argc, char **argv)
{
  if(argc != 2){
    cerr<<"Usage: "<<argv[0]<<" filename"<<endl;
    exit(0);
  }

  try{
    VRawDataFileRead reader(argv[1]);
    VRawEventData event;
    VRawEventParser parser;
    VGPSDecoder GPSDecoder;

    //  cout<<"Counting events, please wait.... "<<endl;
    //    cout<<"SPW Event Count: "<<reader.getEventCount()<<endl;
    //   reader.rewindFile();

    for(uint32_t evt=0;evt<4; evt++){
      reader.getNextEvent(event);
      
      parser.setEvent(&event);
      
      cout<<"--------------------------------------------------------"<<endl;
      cout<<"Data Length:      "<<parser.getDataLength()<<endl;
      cout<<"Event Number:     "<<parser.getEventNumber()<<endl;
      cout<<"Node Number:      "<<(uint32_t) parser.getNode()<<endl;
      cout<<"Tel. Trig. Mask   "<<(uint32_t) parser.getTelTrigMask()<<endl;
      cout<<"Event Type:       "<<(uint32_t) parser.getEventType()<<endl;

      cout<<"Num Channel Hit:  "<<parser.getNumChannelsHit()<<endl;
      cout<<"Num Channel HitB: "<<parser.getNumBitsHit()<<endl;
      cout<<"Max Channels:     "<<parser.getMaxChannels()<<endl;
      cout<<"Num Samples:      "<<parser.getNumSamples()<<endl;

      cout<<"GPS:              0x" << hex <<parser.getGPS0()
          <<" 0x"<<parser.getGPS1()
          <<" 0x"<<parser.getGPS2()<<dec<<endl;
      GPSDecoder.decode(parser.getGPS0(),parser.getGPS1(),parser.getGPS2());
      cout<<GPSDecoder<<endl;


      cout<<"DOT (Status):     0x"<<hex<<parser.getStatus()<<dec<<endl;


      vector<uint32_t> fullChargeVec = parser.getFullChargeVec();
      cout<<"fullChargeVec: ("<<fullChargeVec.size()<<") ";
      for(uint32_t i=0;i<fullChargeVec.size();i++)
        cout<<fullChargeVec[i]<<" ";
      cout<<endl;


      vector<uint32_t> summedSamplesChargeVec = parser.getSummedSamplesChargeVec();
      cout<<"summedSamplesChargeVec: ("<<summedSamplesChargeVec.size()<<") ";
      for(uint32_t i=0;i<summedSamplesChargeVec.size();i++)
        cout<<summedSamplesChargeVec[i]<<" ";
      cout<<endl;


    
      vector<uint32_t> hitVec = parser.getHitVec();
      cout<<"HitVec: ("<<hitVec.size()<<") ";
      for(uint32_t i=0;i<hitVec.size();i++)
        cout<<hitVec[i]<<" ";
      cout<<endl;
    
      vector<bool> fullHitVec = parser.getFullHitVec();
      cout <<"Full Hit Vec: ("<<fullHitVec.size()<<")"<<endl;;
      for(uint32_t i=0; i<fullHitVec.size();i++){
        cout<<" "<<(uint32_t) fullHitVec[i];
        if ((i+1)%10==0) cout<<endl;
      }


      vector<bool> trigVec = parser.getFullTrigVec();
      cout <<"Full Trig Vec: ("<<trigVec.size()<<")" <<endl;
      for(uint32_t i=0; i<trigVec.size();i++){
        cout<<" "<<(uint32_t) trigVec[i];
        if ((i+1)%10==0) cout<<endl;
      }


      for(uint32_t i=0; i<parser.getNumChannelsHit(); i++){
        parser.selectHitChan(i);
        
        cout<<"--- "<<endl;
        cout<<"Hit:     "<<i<<endl;
        cout<<"Chan. Num: "<<parser.getHitID(i)<<endl;
        cout<<"Samples: ";
        for(uint32_t i=0;i<parser.getNumSamples();i++)
          cout<<(uint32_t) parser.getSample(i)<<" ";
        cout<<endl;

//      // test of vector
//      vector<uint8_t> samplesVec = parser.getSamplesVec();
//      for(uint32_t i=0; i<samplesVec.size();i++)
//        cout<<(uint32_t) samplesVec[i]<<" ";
//      cout<<endl;



        cout<<"Charge:   "<<parser.getCharge()<<endl;
        cout<<"Pedestal: "<<parser.getPedestal()<<endl;
        cout<<"HiLo:     "<<parser.getHiLo()<<endl;
      }
      
      cout<<"-------------------"<<endl;
      
      for(uint32_t ct=0;ct<parser.getNumCT();ct++){
        vector<uint32_t> CTVec = parser.getCT(ct);
        cout<<"CT("<<ct<<") ";
        for (uint32_t i=0;i<CTVec.size();i++)
          cout<<hex<<CTVec[i]<<" ";
        cout<<dec<<endl;
      }
    }
  }
  //  catch(VFileOpenException &ex){
  //    cerr<<"File Open Exception!"<<endl;
  //    cerr<<ex<<endl;
  //  } 
  catch(VFileException &ex){
    cerr<<ex<<endl;
  } 
  catch(VIndexOutOfBoundsException &ex){
    cerr<<ex<<endl;
  }
  catch(VSyncException &ex){
    cerr<<ex<<endl;
  }
  catch (const exception &e) {
    cerr<<"Error: "<<e.what()<<endl;
  }


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
