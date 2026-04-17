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
      cout<<"about to get next event."<<endl;
      reader.getNextEvent(event);
      
      parser.setEvent(&event);
      
	  cout<<parser.getEventNumber()<<":"<<endl;
      cout<<"GPS:              0x" << hex <<parser.getGPS0()
          <<" 0x"<<parser.getGPS1()
          <<" 0x"<<parser.getGPS2()<<dec<<endl;
      GPSDecoder.decode(parser.getGPS0(),parser.getGPS1(),parser.getGPS2());
      cout<<GPSDecoder<<endl;
	  
	  cout<<"printed the decoder."<<endl;
    }
	
	cout<<"dude!"<<endl;
  }
  catch(VFileException &ex){
    cerr<<ex<<endl;
  } 
  catch(VIndexOutOfBoundsException &ex){
    cerr<<ex<<endl;
  }
  catch(VSyncException &ex){
    cerr<<ex<<endl;
  }

  cout<<"got to here."<<endl;

  return 0;
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
