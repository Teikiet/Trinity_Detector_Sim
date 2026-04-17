/*
 * VSampleCompressor.h
 * by Filip Pizlo, 2005, 2006
 */

#ifndef V_SAMPLE_COMPRESSOR_H
#define V_SAMPLE_COMPRESSOR_H

#include "VException.h"
#include <iostream>
#include <inttypes.h>

class VSampleCompressor {
private:
    uint8_t *buffer;
    bool hanging_4;
    unsigned num_samples;
    uint8_t *cur;

    unsigned cnt; // purely for debugging

    void push_back(uint8_t val) {
        *cur++=val;
    }
    
    void pop_back() {
        cur--;
    }

    uint8_t &back() {
        return *(cur-1);
    }
    
    uint8_t &at(unsigned i) {
        return buffer[i];
    }

public:
    VSampleCompressor(unsigned num_samples,
                      unsigned num_channels):
        buffer(new uint8_t[(num_samples+1)*num_channels]),
        hanging_4(false),
        num_samples(num_samples),
        cur(buffer),
	cnt(0)
    {}
    
    ~VSampleCompressor() {
        if (buffer!=NULL) {
            delete buffer;
        }
    }
    
    void releaseBuffer() {
        buffer=NULL;
    }
    
    uint8_t *begin() {
        return buffer;
    }
    
    uint8_t *end() {
        return cur;
    }
    
    unsigned size() {
        return cur-buffer;
    }
    
    unsigned nBits() {
        return size()*8-(hanging_4?4:0);
    }
    
    // I is almost always uint8_t*, but we make it a template because
    // sometimes we will use a vector< uint8_t >.
    template< typename I >
    void add(I trace,
             bool xtra_bit) {
        unsigned min=255;
        unsigned max=0;
        for (unsigned j=0;j<num_samples;++j) {
            if (trace[j]<min) {
                min=trace[j];
            }
            if (trace[j]>max) {
                max=trace[j];
            }
        }
        unsigned range;
        if (min>20) {
            min=20;
        }
        range=max-min;
        unsigned bits=0;
        while (range>0) {
            range>>=1;
            bits++;
        }
        if (bits>4) {
            bits=5;
        }
        
        unsigned exp_n_bits=8+(bits==5?8:bits)*num_samples;
        unsigned prev_n_bits=nBits();
        
        uint8_t header=min+bits*21;
        if (xtra_bit) {
            header+=21*6;
        }
	
	//cout<<cnt<<": hanging_4 = "<<hanging_4<<", min = "<<min<<", bits = "<<bits<<", size = "<<size();
        
        if (hanging_4) {
            back()|=header&0xf0;
            push_back(header&0x0f);
        } else {
            push_back(header);
        }
        bool next_hanging_4=false;
        unsigned last_i=size()-1;
        switch (bits) {
        case 0: break;
        case 1: {
            unsigned n=num_samples&~7;
            for (unsigned i=0;i<n;i+=8) {
                push_back(((trace[i+0]-min)<<0)|
                          ((trace[i+1]-min)<<1)|
                          ((trace[i+2]-min)<<2)|
                          ((trace[i+3]-min)<<3)|
                          ((trace[i+4]-min)<<4)|
                          ((trace[i+5]-min)<<5)|
                          ((trace[i+6]-min)<<6)|
                          ((trace[i+7]-min)<<7));
            }
            if (n!=num_samples) {
                push_back(((trace[n+0]-min)<<0)|
                          ((trace[n+1]-min)<<1)|
                          ((trace[n+2]-min)<<2)|
                          ((trace[n+3]-min)<<3));
                next_hanging_4=true;
            }
            break;
        }
        case 2: {
            for (unsigned i=0;i<num_samples;i+=4) {
                push_back(((trace[i+0]-min)<<0)|
                          ((trace[i+1]-min)<<2)|
                          ((trace[i+2]-min)<<4)|
                          ((trace[i+3]-min)<<6));
            }
            break;
        }
        case 3: {
            unsigned n=num_samples&~7;
            for (unsigned i=0;i<n;i+=8) {
                push_back(((trace[i+0]-min)<<0)|
                          ((trace[i+1]-min)<<3)|
                          (((trace[i+2]-min)&3)<<6));
                push_back((((trace[i+2]-min)>>2)&1)|
                          ((trace[i+3]-min)<<1)|
                          ((trace[i+4]-min)<<4)|
                          (((trace[i+5]-min)&1)<<7));
                push_back((((trace[i+5]-min)>>1)&3)|
                          ((trace[i+6]-min)<<2)|
                          ((trace[i+7]-min)<<5));
            }
            if (n!=num_samples) {
                push_back(((trace[n+0]-min)<<0)|
                          ((trace[n+1]-min)<<3)|
                          (((trace[n+2]-min)&3)<<6));
                push_back((((trace[n+2]-min)>>2)&1)|
                          ((trace[n+3]-min)<<1));
                next_hanging_4=true;
            }
            break;
        }
        case 4: {
            for (unsigned i=0;i<num_samples;i+=2) {
                push_back(((trace[i+0]-min)<<0)|
                          ((trace[i+1]-min)<<4));
            }
            break;
        }
        case 5: {
            for (unsigned i=0;i<num_samples;++i) {
                push_back(trace[i]);
            }
            break;
        }
        default:
            V_FAIL("huh?");
        }
        if (hanging_4) {
            if (next_hanging_4) {
                at(last_i)|=back()<<4;
                pop_back();
                hanging_4=false;
            } else {
                at(last_i)|=back()&0xf0;
                back()&=0xf;
                hanging_4=true;
            }
        } else {
            hanging_4=next_hanging_4;
        }
	
	//cout<<", size = "<<size()<<", hanging_4 = "<<hanging_4<<endl;
	
	cnt++;
        
        V_ASSERT(exp_n_bits==nBits()-prev_n_bits);
    }
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
