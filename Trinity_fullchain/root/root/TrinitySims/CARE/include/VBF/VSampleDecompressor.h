/*
 * VSampleDecompressor.h
 * by Filip Pizlo, 2005, 2006
 */

#ifndef V_SAMPLE_DECOMPRESSOR_H
#define V_SAMPLE_DECOMPRESSOR_H

#include "VSampleDecompressor.h"
#include <inttypes.h>
#include <iostream>
#include <algorithm>

class VSampleDecompressor {
private:
    uint8_t *begin,*cur,*end;
    uint8_t *bounce;
    bool hanging_4;
    unsigned num_samples;
    
public:
    VSampleDecompressor(unsigned num_samples,
                        uint8_t *cur,
                        uint8_t *end):
        begin(cur),
        cur(cur),
        end(end),
        bounce(new uint8_t[num_samples]),
        hanging_4(false),
        num_samples(num_samples)
    {}
    
    ~VSampleDecompressor() {
        delete[] bounce;
    }
    
    uint8_t *getCur() {
        return cur;
    }
    
    unsigned size() {
        return cur-begin;
    }
    
    template< typename T >
    void get(T targ,
             bool &xtra_bit) {
        V_ASSERT(cur+1<=end);
        unsigned header;
        if (hanging_4) {
            header=(*(cur-1)&0xf0)|(*cur&0x0f);
            cur++;
        } else {
            header=*cur++;
        }
        unsigned min=header%21;
        unsigned bits=(header/21)%6;
        if (header/(21*6)) {
            xtra_bit=true;
        } else {
            xtra_bit=false;
        }
        switch (bits) {
        case 0: {
            for (unsigned i=num_samples;i-->0;) {
                targ[i]=min;
            }
            break;
        }
        case 5: {
            V_ASSERT(cur+num_samples<=end);
	    std::copy(cur,cur+num_samples,targ);
            if (hanging_4) {
                targ[num_samples-1]&=0xf;
                targ[num_samples-1]|=*(cur-1)&0xf0;
            }
            cur+=num_samples;
            break;
        }
        case 1:
        case 2:
        case 3:
        case 4: {
            uint8_t *buf=bounce;
            unsigned n;
            if (bits==1 || bits==3) {
                if (bits==1) {
                    n=num_samples/8;
                } else {
                    n=(num_samples*3)/8;
                }
                if (hanging_4) {
                    memcpy(bounce,cur,n);
                    if (num_samples&4) {
                        bounce[n]=(*(cur-1)>>4)&0xf;
                        hanging_4=false;
                    } else {
                        bounce[n-1]&=0xf;
                        bounce[n-1]|=*(cur-1)&0xf0;
                    }
                } else {
                    buf=cur;
                    if (num_samples&4) {
                        hanging_4=true;
                        V_ASSERT(cur+1<=end);
                        cur++;
                    }
                }
            } else {
                if (bits==2) {
                    n=num_samples/4;
                } else {
                    n=num_samples/2;
                }
                if (hanging_4) {
                    memcpy(bounce,cur,n);
                    bounce[n-1]&=0xf;
                    bounce[n-1]|=*(cur-1)&0xf0;
                } else {
                    buf=cur;
                }
            }
            V_ASSERT(cur+n<=end);
            cur+=n;
            switch (bits) {
            case 1: {
                for (unsigned i=0;i<n;++i) {
                    *targ++=min+((buf[i]>>0)&1);
                    *targ++=min+((buf[i]>>1)&1);
                    *targ++=min+((buf[i]>>2)&1);
                    *targ++=min+((buf[i]>>3)&1);
                    *targ++=min+((buf[i]>>4)&1);
                    *targ++=min+((buf[i]>>5)&1);
                    *targ++=min+((buf[i]>>6)&1);
                    *targ++=min+((buf[i]>>7)&1);
                }
                if (num_samples&4) {
                    *targ++=min+((buf[n]>>0)&1);
                    *targ++=min+((buf[n]>>1)&1);
                    *targ++=min+((buf[n]>>2)&1);
                    *targ++=min+((buf[n]>>3)&1);
                }
                break;
            }
            case 2: {
                for (unsigned i=0;i<n;++i) {
                    *targ++=min+((buf[i]>>0)&3);
                    *targ++=min+((buf[i]>>2)&3);
                    *targ++=min+((buf[i]>>4)&3);
                    *targ++=min+((buf[i]>>6)&3);
                }
                break;
            }
            case 3: {
                for (unsigned i=0;i+2<n;i+=3) {
                    *targ++=min+((buf[i+0]>>0)&7);
                    *targ++=min+((buf[i+0]>>3)&7);
                    *targ++=min+(((buf[i+0]>>6)&3)|((buf[i+1]&1)<<2));
                    *targ++=min+((buf[i+1]>>1)&7);
                    *targ++=min+((buf[i+1]>>4)&7);
                    *targ++=min+(((buf[i+1]>>7)&1)|((buf[i+2]&3)<<1));
                    *targ++=min+((buf[i+2]>>2)&7);
                    *targ++=min+((buf[i+2]>>5)&7);
                }
                if (num_samples&4) {
                    *targ++=min+((buf[n-1]>>0)&7);
                    *targ++=min+((buf[n-1]>>3)&7);
                    *targ++=min+(((buf[n-1]>>6)&3)|((buf[n-0]&1)<<2));
                    *targ++=min+((buf[n-0]>>1)&7);
                }
                break;
            }
            case 4: {
                for (unsigned i=0;i<n;++i) {
                    *targ++=min+((buf[i]>>0)&15);
                    *targ++=min+((buf[i]>>4)&15);
                }
                break;
            }
            default:
                V_FAIL("huh?");
            }
            break;
        }
        default:
            V_FAIL("huh?");
        }
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
