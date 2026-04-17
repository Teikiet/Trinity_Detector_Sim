/*
 * MakeBogusFileWithCorsikaSimulationData.cpp -- makes a bogus VBF file with all zeroes in all fields;
 * except for corsika simulation data (shower ID and first interaction height).
 * by Henrike Fleischhack, 2015. Based on
 * MakeBogusFile.cpp -- makes a bogus VBF file with all zeroes in all fields.
 * by Filip Pizlo, 2004
 */

// these are the headers you'll need for writing VBF files.
#include <VBF/VBankFileWriter.h>
#include <VBF/VPacket.h>
#include <VBF/VArrayEvent.h>
#include <VBF/VDatum.h>

// include the simulation data structure
#include <VBF/VSimulationData.h>
#include <VBF/VCorsikaSimulationData.h>

// include the configuration mask utilities, which give us parseConfigMask()
#include <VBF/VConfigMaskUtil.h>

// some system headers; you may or may not want these depending on how you
// use the library
#include <iostream>
#include <exception>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

// declare that we're using the VConfigMaskUtil namespace, which gives us
// easy access to parseConfigMask().
using namespace VConfigMaskUtil;

int main() {
    try {
        // open a file for writing, specifying that the filename is 'bogus.vbf'
        // that the run number is 400, and that the configuration mask
        // includes the first two telescopes.  note that you'll need to
        // provide the run number to the array event and array trigger below as
        // well.
        VBankFileWriter writer("bogus.vbf",
                               400,
                               parseConfigMask("0,1"));
        
        // write a hundred events.  note that your hard drive is pretty much the
        // limit here, since you can store up to 2^32 events in a VBF file!
        for (unsigned i=0;
             i<100;
             ++i) {
            VPacket *packet=new VPacket();
            
            // first create the array event.  notice that the array event also
            // has to know about the run number.
            VArrayEvent *ae=new VArrayEvent(400);
            
            // now loop, creating two events - one for each of the two telescopes!
            for (unsigned j=0;
                 j<2;
                 ++j) {
                VEvent *event=new VEvent();
                
		// by default, an event has 0 channels and 0 samples.  here
		// I resize it to have 64 samples and 499 channels.  note that
		// this means that 499 channels passed zero suppression.
		event->resizeChannelData(64,499);
		
		// by default, the max num channels is 0.  it must be set to the
		// actual number of channels recognized by the DACQ.  here we
		// set it to 499
		event->resizeChannelBits(499);

		// by default, the event contains 0 clock trigger boards.  here
		// I make it 1
		event->resizeClockTrigData(1);
		
		// note that you call call those resize methods more than once.
		// in particular, you can keep adding channels, samples, or clock
		// trigger boards with repeated calls to the resize methods.
		// this may not give you great performance, however.
                
                // set the event number to i
                event->setEventNumber(i);
                
                // set the node number (telescope number) to j
                event->setNodeNumber(j);
                
                // now I will set all of the fields to 0, except for the hit pattern,
                // which will be set so all of the bits are 1 (this means that no channels
                // got zero-suppressed).  if you're creating events from
                // a simulation, you can use this code as a template!
                
		event->getGPSTime()[0]=0;
		event->getGPSTime()[1]=0;
		event->getGPSTime()[2]=0;
		event->getGPSTime()[3]=0;
		event->getGPSTime()[4]=0;
                
                event->setGPSYear(0);
                
                event->setEventTypeCode(0);
                
                event->setFlags(1); // enable compression

		// specify that no channels triggered.
                for (unsigned k=0;
                     k<event->getMaxNumChannels();
                     ++k) {
		    event->setTriggerBit(k,false);
                }
		
		// specify that all channels passed zero suppression
                for (unsigned k=0;
                     k<event->getMaxNumChannels();
                     ++k) {
		    event->setHitBit(k,true);
                }
                
                for (unsigned k=0;
                     k<event->getNumChannels();
                     ++k) {
                    event->setCharge(k,0);
		    event->setPedestal(k,0);
		    event->setHiLo(k,false);
                    
                    for (unsigned l=0;
                         l<event->getNumSamples();
                         ++l) {
			if ((k==3 || k==6) && l>=7) {
			    event->setSample(k,l,10);
			} else {
			    event->setSample(k,l,i%2);
			}
                    }
                }
                
                for (unsigned k=0;
                     k<event->getNumClockTrigBoards();
                     ++k) {
                    for (unsigned l=0;
                         l<7;
                         ++l) {
			event->getClockTrigData(k)[l]=0;
                    }
                }
                
                // add the event to the array event!
                ae->addEvent(event);
            }
            
            // now create an array trigger
            VArrayTrigger *at=new VArrayTrigger();
            
	    // by default, the array trigger contains 0 subarray telescope
	    // entries.  here I set it to 7.
	    at->resizeSubarrayTelescopes(7);

	    // by default, the array trigger contains 0 triggered telescope
	    // entries.  here I set it to 7.
	    at->resizeTriggerTelescopes(7);
            
            // set the event number to i
            at->setEventNumber(i);
            
            // set the node number to 255.
            at->setNodeNumber(255);
            
            // the array trigger also needs to know about the run number.
            at->setRunNumber(400);

            // now I will set all of the fields to 0.  if you're creating events from
            // a simulation, you can use this code as a template!
            at->getGPSTime()[0]=0;
            at->getGPSTime()[1]=0;
            at->getGPSTime()[2]=0;
            at->getGPSTime()[3]=0;
            at->getGPSTime()[4]=0;
            
            at->setGPSYear(0);
            
            at->setEventTypeCode(0);
            
            at->setFlags(0);
            
            at->setATFlags(0);
            
            for (unsigned j=0;
                 j<at->getNumSubarrayTelescopes();
                 ++j) {
                // have to set the telescope ID that this record corresponds to.
                // in this case, the record number and telescope ID happen to
                // be the same
                at->setSubarrayTelescopeId(j,j);
                
                at->setAltitude(j,0.0);
                at->setAzimuth(j,0.0);
                at->setTDCTime(j,0);
		at->setSpecificEventTypeCode(j,0);
		at->setShowerDelay(j,0);
		at->setCompDelay(j,0);
            }
            
            // now add the array trigger to the array event
            ae->setTrigger(at);
            
            // put the array event into the packet
            packet->putArrayEvent(ae);
            
            // now construct the simulation data.  I'll set the fields to 0, but
            // you can set them to whatever you like!

            VSimulationData *simu_data=
                new VSimulationData(0, // id
                                    0.0,   // e
                                    0.0,   // theta
                                    0.0,   // phi
				    0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 );
            
            // and put the simulation data into the packet
            packet->putSimulationData(simu_data);

	    //corsika simulation data. Set showerID=eventNumber+1 and first interaction height = 10 km +100m*ShowerID, depth=50-0.1*ShowerID, CorsikaRunID=ShowerID/10
	    VCorsikaSimulationData * cors_data = new VCorsikaSimulationData( 10.e3+100.0*(i+1), 50.-0.1*(i+1), (i+1)/10, (i+1) );
	    packet->putCorsikaSimulationData( cors_data );

            
            // finally, write the packet into the file
            writer.writePacket(i, packet);
            
	    cerr<<"wrote packet "<<i<<endl;

            // dispose of the packet, so that we don't leak memory
            delete packet;
        }
        
        // finish up.  this creates the index and writes the checksum.
        writer.finish();
        
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
