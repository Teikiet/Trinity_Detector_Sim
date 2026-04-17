/*
 * PrintCorsikaSimulationData.cpp -- prints corsika simulation data for the given event number
 * by Henrike Fleischhack, 2015
 * PrintSpecificEvent.cpp -- prints the event with the given event number for
 *                           the specified telescope
 * by Filip Pizlo, 2004
 */

// include necessary VBF header files.  these are the header files you'll need
// for reading events.
#include <VBF/VBankFileReader.h>
#include <VBF/VPacket.h>
#include <VBF/VArrayEvent.h>
#include <VBF/VDatum.h>
#include <VBF/VGPSDecoder.h>
#include <VBF/VCorsikaSimulationData.h>

// some system headers; you may or may not want these depending on how you
// use the library
#include <iostream>
#include <exception>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

// a helpful usage function
static void usage() {
    cerr<<"Usage: PrintCorsikaSimulationData <filename> <event number>"<<endl;
    exit(1);
}

static void printCorsika(VCorsikaSimulationData * corsika) {
    // boring function to print out the event.  if you're trying to analyze
    // data, this is the function that you'd probably want to replace with
    // analysis code.
    
    cout<<"-----------------------------------------------"<<endl;
    
    cout<<"Run number: "<<corsika->fRunNumber <<endl;
    cout<<"Event number: "<<corsika->fEventNumber <<endl;    
    cout<<"First Interaction height: "<<corsika->fFirstInteractionHeight<< " m" << endl;
    cout<<"First Interaction depth: "<<corsika->fFirstInteractionDepth<< " g/cm^2" << endl;
    cout<<"Run ID: "<<corsika->fCorsikaRunID<<endl;
    cout<<"Shower ID: "<<corsika->fShowerID<<endl;
      
}

int main(int c,char **v) {
    try {
        if (c!=3) {
            usage();
        }
        
        // extract the arguments
        const char *filename=v[1];
        unsigned event_num;
        
        
        if (sscanf(v[2],"%u",&event_num)!=1) {
            cerr<<v[2]<<" is not a valid event number."<<endl;
            usage();
        }
        
        // create a reader for the bank file.  this will open the file in a
        // read-only fashion and will memory-map the index, allowing you
        // random access.
        VBankFileReader reader(filename);
        
        if (event_num>=reader.numPackets()) {
            cerr<<"You requested event number "<<event_num<<", but the largest event number in the file"<<endl
                <<"is "<<(reader.numPackets()-1)<<"."<<endl;
            return 1;
        }
        
        // read the packet with the given event number.
        VPacket *packet=reader.readPacket(event_num);
        
        if (!packet->hasCorsikaSimulationData()) {
            cerr<<"There is no corsika simulation data for event number "<<event_num<<"."<<endl;
            return 1;
        }
        
        VCorsikaSimulationData *data=packet->getCorsikaSimulationData();
        
        if (data==NULL) {
            cerr<<"Corsika simulation data should be there, but could not be read for event number "<< event_num<<"."<<endl;
            return 1;
        }
        
        printCorsika(data);
        
        delete packet;

    } catch (const exception &e) {
        cerr<<"Error: "<<e.what()<<endl;
        return 1;
    }
    
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
