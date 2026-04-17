/*
 * VConfigMaskUtil.h -- utility functions for dealing with configuration masks
 * by Filip Pizlo, 2004
 */

#ifndef V_CONFIG_MASK_UTIL_H
#define V_CONFIG_MASK_UTIL_H

#include "VException.h"
#include "VConstants.h"
#include <vector>

namespace VConfigMaskUtil {
    
    class Exception: public ::VException {};
    
    class BadConfigMaskStringException: public Exception {
        public:
            BadConfigMaskStringException(const std::string &msg="") {
                setStrings("The given configuration mask is badly formatted",
                           msg);
            }
    };
    
    inline std::vector< bool > fromDAQMask(unsigned mask) {
        std::vector< bool > result;
        for (unsigned i=0;
             i<V_NUM_TELESCOPES;
             ++i) {
            result.push_back((bool)(mask&(1<<i)));
        }
        return result;
    }

    inline unsigned toDAQMask(const std::vector< bool > &mask) {
	unsigned result=0;
	for (unsigned i=0;
	     i<V_NUM_TELESCOPES && i<mask.size();
	     ++i) {
	    if (mask[i]) {
		result|=(1<<i);
	    }
	}
	return result;
    }
    
    inline std::vector< bool > parseConfigMask(const char *str) {
        char buf[4];    // largest config mask (255) requires 4 chars to be
                        // represented as a C-string
        
        const char *ptr=str,*end=str+strlen(str);
        
        std::vector< bool > result;
        
        for (unsigned i=0;i<255;++i) {
            result.push_back(false);
        }
        
        while (ptr<end) {
            const char *comma=strchr(ptr,',');
            if (comma==NULL) {
                comma=ptr+strlen(ptr);
            }
            
            if (comma-ptr>3) {
                throw BadConfigMaskStringException(
                    "The size of any given telescope ID must be 3 bytes or "
                    "less");
            }
            
            if (comma-ptr==0) {
                throw BadConfigMaskStringException(
                    "Empty telescope IDs are invalid");
            }
            
            memcpy(buf,ptr,comma-ptr);
            buf[comma-ptr]=0;
            
            unsigned value;
            if (sscanf(buf,"%u",&value)!=1 ||
                value>=255) {
                throw BadConfigMaskStringException(
                    "Telescope IDs should be unsigned integers");
            }
            
            result[value]=true;
            
            ptr=comma+1;
        }
        
        return result;
    }
    
    inline std::string configMaskToString(
            const std::vector< bool > &config_mask,
            bool allow_empty=true) {
        std::ostringstream buf;
        bool first=true;
        for (unsigned i=0;
             i<config_mask.size();
             ++i) {
            if (config_mask[i]) {
                if (first) {
                    first=false;
                } else {
                    buf<<",";
                }
                buf<<i;
            }
        }
        if (allow_empty && buf.str().empty()) {
            return "empty";
        }
        return buf.str();
    }
    
    inline unsigned cardinality(const std::vector< bool > &a) {
	unsigned result=0;
	for (unsigned i=a.size();i-->0;) {
	    if (a[i]) {
		result++;
	    }
	}
	return result;
    }

    inline unsigned ith(const std::vector< bool > &mask,
			unsigned i) {
	for (unsigned j=0;j<mask.size();++j) {
	    if (mask[j]) {
		if (i==0) {
		    return j;
		}
		--i;
	    }
	}
	return mask.size();
    }

    inline std::vector< bool > intersect(const std::vector< bool > &a,
					 const std::vector< bool > &b) {
	std::vector< bool > result;
	for (unsigned i=0;i<a.size() && i<b.size();++i) {
	    if (a[i] && b[i]) {
		while (result.size()<i) {
		    result.push_back(false);
		}
		result.push_back(true);
	    }
	}
	return result;
    }

    inline std::vector< bool > unify(const std::vector< bool > &a,
				     const std::vector< bool > &b) {
	std::vector< bool > result;
	while (result.size()<a.size() || result.size()<b.size()) {
	    result.push_back(false);
	}
	for (unsigned i=0;i<a.size();++i) {
	    if (a[i]) {
		result[i]=true;
	    }
	}
	for (unsigned i=0;i<b.size();++i) {
	    if (b[i]) {
		result[i]=true;
	    }
	}
	return result;
    }
    
    inline bool masksEqual(const std::vector< bool > &a,
                           const std::vector< bool > &b) {
        std::vector< bool >::const_iterator i=a.begin(),
                                            j=b.begin();
        
        while (i!=a.end() && j!=b.end()) {
            if (*i != *j) {
                return false;
            }
            
            ++i;
            ++j;
        }
        
        if (i==a.end()) {
            while (j!=b.end()) {
                if (*j) {
                    return false;
                }
                
                ++j;
            }
        } else {
            while (i!=a.end()) {
                if (*i) {
                    return false;
                }
                
                ++i;
            }
        }
        
        return true;
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
