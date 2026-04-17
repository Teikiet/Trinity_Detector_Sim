/*
 * VBankFileReader.cpp -- implementation of VBF reader
 * by Filip Pizlo, 2004
 */

#include "VBankFileReader.h"
#include "VBFUtil.h"
#include "VArrayEvent.h"
#include "VSimulationData.h"
#include "VBankFileBzip2Reader.h"
#include "VBankFileGzipReader.h"
#include "VEventOverflow.h"

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "Adler32.h"
#include <iostream>

using namespace std;
using namespace VBFUtil;

void VBankFileReader::myRead(word64 offset,char *buf,uword32 len) {
    int res=::pread(fd,buf,len,(off_t)offset);
    if (res<0) {
        throw VSystemException("In VBankFileReader::myRead()");
    }
    if ((uword32)res!=len) {
        throw VBankFileReaderBadFormatException(
            "In VBankFileReader::myRead(): unexpected end of file");
    }
}

uint8_t VBankFileReader::myReadByte(word64 offset) {
    uint8_t result;
    myRead(offset,(char*)&result,1);
    return result;
}

uword32 VBankFileReader::myReadWord32(word64 offset) {
    char buf[4];
    myRead(offset,buf,4);
    return bufToWord32(buf);
}

uword64 VBankFileReader::myReadWord64(word64 offset) {
    char buf[8];
    myRead(offset,buf,8);
    return bufToWord64(buf);
}

uword32 VBankFileReader::myWrite(word64 offset,
                                 uword32 adler,
                                 const char *buf,
                                 uword32 len) {
    int res=::pwrite(fd,buf,len,(off_t)offset);
    if (res<0) {
        throw VSystemException("In VBankFileReader::myWrite()");
    }
    V_ASSERT((uword32)res==len);    // should always be true
    return ::vbf_adler32(adler,(const unsigned char*)buf,len);
}

uword32 VBankFileReader::myWrite(word64 offset,
                                 uword32 adler,
                                 uword32 value) {
    char buf[4];
    wordToBuf(value,buf);
    return myWrite(offset,adler,buf,4);
}

uword32 VBankFileReader::myWrite(word64 offset,
                                 uword32 adler,
                                 uword64 value) {
    char buf[8];
    wordToBuf(value,buf);
    return myWrite(offset,adler,buf,8);
}

void VBankFileReader::myWrite(word64 offset,const char *buf,uword32 len) {
    int res=::pwrite(fd,buf,len,(off_t)offset);
    if (res<0) {
        throw VSystemException("In VBankFileReader::myWrite()");
    }
    V_ASSERT((uword32)res==len);    // should always be true
}

void VBankFileReader::myWrite(word64 offset,uword32 value) {
    char buf[4];
    wordToBuf(value,buf);
    myWrite(offset,buf,4);
}

void VBankFileReader::myWrite(word64 offset,uword64 value) {
    char buf[8];
    wordToBuf(value,buf);
    myWrite(offset,buf,8);
}

void VBankFileReader::skipNextPacket() {
    char buf[4];
    myRead(packet_offset,buf,4);
    if (memcmp(buf,"VPCK",4)) {
        throw VBankFileReaderBadFormatException(
            "In VBankFileReader::skipNextPacket()");
    }
    
    packet_offset+=myReadWord32(packet_offset+4);
    packet_index++;
}

void VBankFileReader::unmapIndex() {
    if (index!=NULL) {
        ::munmap(index_base,index-index_base+num_packets*8);
    }
}

VBankFileReader::VBankFileReader(const std::string &filename,
                                 bool map_index,
                                 bool read_only):
    read_only(read_only),
    stream(NULL),
    next_packet(NULL)
{
    ostringstream buf;
    buf<<"In VBankFileReader::VBankFileReader() for \""
       <<filename<<"\"";

    fd=::open(filename.c_str(),read_only?O_RDONLY:O_RDWR);
    if (fd<0) {
        throw VSystemException(buf.str());
    }
    
    char magic_buf[4];
    myRead(0,magic_buf,2);
    if (!memcmp(magic_buf,"BZ",2)) {
	if (!read_only) {
	    throw VBankFileReaderStreamedException
		("Must use read only mode for bzip2 files");
	}
	// FIXME: memory leak if this errors out.
	stream=new VBankFileBzip2Reader(this,fd);
	stream->initialize();
	run_number=stream->getRunNumber();
	config_mask=stream->getConfigMask();
	next_packet=stream->readPacket();
    } else if (magic_buf[1]==(char)0x8b &&
	       magic_buf[0]==(char)0x1f) {
	if (!read_only) {
	    throw VBankFileReaderStreamedException
		("Must use read only mode for gzip files");
	}
	// FIXME: memory leak if this errors out.
	stream=new VBankFileGzipReader(this,fd);
	stream->initialize();
	run_number=stream->getRunNumber();
	config_mask=stream->getConfigMask();
	next_packet=stream->readPacket();
    } else {
	myRead(0,magic_buf,4);
	if (memcmp(magic_buf,"VBFF",4)) {
	    throw VBankFileReaderBadMagicNumberException(buf.str());
	}
	
	if (myReadWord32(4)!=0) {
	    throw VBankFileReaderBadVersionNumberException(buf.str());
	}
	
	run_number=myReadWord32(8);
	
	off_t file_size=::lseek(fd,0,SEEK_END);
	if (file_size<0) {
	    throw VSystemException(buf.str());
	}

	pre_footer_size=myReadWord64(48);
	if (pre_footer_size==0) {
	    pre_footer_size=(uword64)file_size;
	} else if (pre_footer_size>(uword64)file_size) {
	    // the file is truncated.
	    pre_footer_size=file_size;
	} else if (pre_footer_size<56) {
	    throw VBankFileReaderBadFormatException
		("The pre footer size is not 0 and is less than 56: "
		 + buf.str());
	}
	
	for (unsigned i=0;
	     i<32;
	     ++i) {
	    uint8_t value=myReadByte(12+i);
	    for (unsigned j=0;
		 j<(i<31?8:7);
		 ++j) {
		config_mask.push_back((bool)(value&(1<<j)));
	    }
	}
	
	packet_offset=56;
    }
    
    packet_index=0;
    
    index=NULL;
    
    if (map_index) {
	if (!mapIndex()) {
	    throw VBankFileReaderNoIndexException
		("User asked for an index but there is none: "+buf.str());
	}
    }

    putBankBuilder(VGetArrayEventBankName(),
                   VGetArrayEventBankBuilder());
    putBankBuilder(VGetSimulationHeaderBankName(),
		   VGetSimulationHeaderBankBuilder());
    putBankBuilder(VGetSimulationDataBankName(),
                   VGetSimulationDataBankBuilder());
    putBankBuilder(VGetEventOverflowBankName(),
		   VGetEventOverflowBankBuilder());
    putBankBuilder(VGetKascadeSimulationHeaderBankName(),
		   VGetKascadeSimulationHeaderBankBuilder());
    putBankBuilder(VGetCorsikaSimulationDataBankName(),
		   VGetCorsikaSimulationDataBankBuilder());
    putBankBuilder(VGetKascadeSimulationDataBankName(),
		   VGetKascadeSimulationDataBankBuilder());
    putBankBuilder(VGetChiLASimulationHeaderBankName(),
		   VGetChiLASimulationHeaderBankBuilder());
    putBankBuilder(VGetChiLASimulationDataBankName(),
		   VGetChiLASimulationDataBankBuilder());
}

VBankFileReader::~VBankFileReader() {
    if (stream!=NULL) {
	delete stream;
    }
    if (next_packet!=NULL) {
	delete next_packet;
    }
    unmapIndex();
    ::close(fd);
}

void VBankFileReader::resetBankBuilders() {
    bank_builders.clear();
    final_bank_builders.clear();
}

void VBankFileReader::putBankBuilder(const VBankName &name,
                                     VBankBuilder *bank_builder) {
    V_ASSERT(bank_builder->canRead(name));
    bank_builders[name]=bank_builder;
}

void VBankFileReader::addFinalBankBuilder(VBankBuilder *bank_builder) {
    final_bank_builders.push_back(bank_builder);
}

VBankBuilder *VBankFileReader::getBankBuilder(const VBankName &name) {
    bank_builder_map::iterator i=bank_builders.find(name);
    if (i==bank_builders.end()) {
	for (unsigned i=final_bank_builders.size();i-->0;) {
	    if (final_bank_builders[i]->canRead(name)) {
		return final_bank_builders[i];
	    }
	}
        return NULL;
    }
    return i->second;
}

bool VBankFileReader::mapIndex() {
    if (stream!=NULL) {
	throw VBankFileReaderStreamedException
	    ("Cannot do mapIndex()");
    }

    unmapIndex();
    
    // check if there is an index
    // (note: nowhere in this class do we rely on the file pointer being in
    // any particular place, since we're always doing preads and mmaps. so,
    // the most convenient way of checking the length of the file is with
    // an lseek.)
    off_t size=::lseek(fd,0,SEEK_END);
    if (size<0) {
        throw VSystemException(
            "In VBankFileReader::mapIndex(), while doing an lseek() to "
            "determine the file's size");
    }
    if (size==(off_t)pre_footer_size) {
        // there is no index
        index=NULL;
        return false;
    }
    
    if (size<(off_t)pre_footer_size+4) {
        std::ostringstream buf;
        buf<<"The actual size of the given VBF file is less than "
             "four bytes longer than the pre-footer size, "
             "indicating that the footer is smaller than the "
             "minimal size: "
           <<"file size = "<<size<<"; pre footer size = "<<pre_footer_size;
        throw VBankFileReaderBadFormatException(buf.str());
    }
    
    // read the number of packets
    num_packets=myReadWord32(pre_footer_size);
    
    // make sure that the num_packets makes sense
    if (pre_footer_size+num_packets*8+4 > (uword64)size) {
        throw VBankFileReaderBadFormatException(
            "The file is smaller than what the num packets and pre "
            "footer size fields indicate");
    } else if (pre_footer_size+num_packets*8+4 < (uword64)size) {
        // warn the user that the file is too big?...  for now, we will
        // ignore this condition
    }
    
    uword64 true_offset=pre_footer_size+4;
    true_offset&=~(getpagesize()-1);
    
    unsigned extra=pre_footer_size+4-true_offset;
    
    index_base=(char*)::mmap(NULL,
			     extra+num_packets*8,
			     PROT_READ,
			     MAP_PRIVATE,
			     fd,
			     true_offset);
    if (index_base==(void*)-1) {
        throw VSystemException(
            "In VBankFileReader::mapIndex(), while doing an mmap() to "
            "map the index");
    }
    
    index=index_base+extra;
    
    return true;
}

bool VBankFileReader::hasChecksum() {
    if (stream!=NULL) {
	return false;
    }
    // only way to know is to read pre footer size from the file
    return myReadWord64(48)!=0;
}

uword32 VBankFileReader::numPackets() const {
    if (!hasIndex()) {
        throw VBankFileReaderNoIndexException();
    }
    return num_packets;
}

void VBankFileReader::streamSkipTo(uword32 index) {
    while (packet_index<index) {
	///cerr<<"skipping!"<<endl;
	if (next_packet!=NULL) {
	    delete next_packet;
	}
	next_packet=stream->readPacket();
	packet_index++;
    }
}

bool VBankFileReader::hasPacket(uword32 index) {
    if (hasIndex()) {
        return index<num_packets;
    }
    if (index<packet_index) {
        throw VBankFileReaderNoIndexException();
    }
    if (stream!=NULL) {
	streamSkipTo(index);
	return next_packet!=NULL;
    } else {
	while ((uword64)packet_offset<pre_footer_size &&
	       packet_index<index) {
	    skipNextPacket();
	}
	return (uword64)packet_offset<pre_footer_size;
    }
}

VPacket *VBankFileReader::readPacket(uword32 index) {
    if (!hasIndex()) {
	if (index<packet_index) {
	    throw VBankFileReaderNoIndexException();
	}
    }
    
    if (stream!=NULL) {
	streamSkipTo(index);
	if (next_packet==NULL) {
	    throw VBankFileReaderIndexOutOfBoundsException();
	} else {
	    VPacket *result=next_packet;
	    next_packet=NULL;
	    return result;
	}
    }

    off_t offset;
    
    if (index!=packet_index) {
        if (hasIndex()) {
            if (index>=num_packets) {
                throw VBankFileReaderIndexOutOfBoundsException();
            }
            offset=(off_t)bufToWord64(this->index+(index*8));
        } else {
            while ((uword64)packet_offset<pre_footer_size &&
                   packet_index<index) {
                skipNextPacket();
            }
            offset=(off_t)packet_offset;
        }
    } else {
        offset=(off_t)packet_offset;
    }
    
    // NOTE: this condition only happens when hasIndex()==false, but we
    // put it out here because it might happen when index==packet_index
    if ((uword64)offset>=pre_footer_size) {
        throw VBankFileReaderIndexOutOfBoundsException();
    }
    
    char buf[4];
    myRead(offset,buf,4);
    if (memcmp(buf,"VPCK",4)) {
        throw VBankFileReaderBadFormatException(
            "In VBankFileReader::readPacket()");
    }
    
    uword32 packet_size=myReadWord32(offset+4);
    
    packet_offset=offset+packet_size;
    packet_index=index+1;
    
    VPacket *result=new VPacket();
    
    // FIXME: memory leak on error!
    
    // now we can actually parse the banks
    offset+=8;
    while (offset<packet_offset) {
        char name_buf[8];
        myRead(offset,name_buf,8);
        
	uword32 version=myReadWord32(offset+8);

        VBankName bank_name(name_buf);
        if (result->has(bank_name)) {
            throw VBankFileReaderBadFormatException(
                "In VBankFileReader::readPacket(): duplicate bank name");
        }
        
        uword32 bank_size=myReadWord32(offset+12);
        VBankBuilder *builder=getBankBuilder(bank_name);
        if (builder!=NULL) {
            result->put(bank_name,
                builder->readBankFromFile(run_number,
                                          index,
                                          bank_name,
					  version,
                                          bank_size-16,
                                          offset+16,
                                          fd));
        }
        
        offset+=bank_size;
    }
    
    return result;
}

void VBankFileReader::resetSequentialRead() {
    if (stream==NULL) {
	packet_offset=56;
    } else {
	stream->reset();
	stream->initialize();
    }
    packet_index=0;
}

uword32 VBankFileReader::generateIndexAndChecksum() {
    if (read_only) {
        throw VBankFileReaderReadOnlyException(
            "Attempted to generateIndexAndChecksum() on read-only file");
    }
    
    unmapIndex();
    
    uword32 adler=::vbf_adler32(0,NULL,0);
    
    size_t buf_size=56;
    char *buf=(char*)::malloc(56);
    if (buf==NULL) {
        throw bad_alloc();
    }
    
    try {
        myRead(0,buf,44);
        ::bzero(buf+44,12);
        
        adler=::vbf_adler32(adler,(const unsigned char*)buf,56);
    
        vector< off_t > my_index;
        
        my_index.push_back(56);
        
        while ((uword64)my_index.back() < pre_footer_size) {
            myRead(my_index.back(),buf,8);
            
            if (memcmp(buf,"VPCK",4)) {
                throw VBankFileReaderBadFormatException(
                    "In VBankFileReader::generateIndexAndChecksum()");
            }
            
            adler=::vbf_adler32(adler,(const unsigned char*)buf,8);
            
            uword32 packet_body_size=bufToWord32(buf+4)-8;
            
            if (buf_size<packet_body_size) {
                buf_size=packet_body_size;
                buf=(char*)::realloc(buf,packet_body_size);
                if (buf==NULL) {
                    throw bad_alloc();
                }
            }
            
            myRead(my_index.back()+8,buf,packet_body_size);
            adler=::vbf_adler32(adler,
				(const unsigned char*)buf,
				packet_body_size);
            
            my_index.push_back(my_index.back()+8+packet_body_size);
        }
        
        my_index.pop_back();    // last entry is for a packet that don't exist
        
        adler=myWrite(pre_footer_size,adler,(uword32)my_index.size());
        
        for (unsigned i=0;
             i<my_index.size();
             ++i) {
            adler=myWrite(pre_footer_size+4+i*8,adler,(uword64)my_index[i]);
        }
        
        myWrite(44,adler);
        myWrite(48,pre_footer_size);
        ::ftruncate(fd,pre_footer_size+4+my_index.size()*8);
        
        ::free(buf);
    } catch (...) {
        ::free(buf);
        throw;
    }
    
    V_ASSERT(mapIndex());
    
    return adler;
}

uword32 VBankFileReader::getChecksum() {
    if (!hasChecksum()) {
        throw VBankFileReaderNoChecksumException();
    }
    return myReadWord32(44);
}

uword32 VBankFileReader::calculateChecksum() {
    if (stream!=NULL) {
	throw VBankFileReaderStreamedException
	    ("Can't calculateChecksum()");
    }

    uword32 adler=::vbf_adler32(0,NULL,0);
    
    char *buf=new char[65536];
    try {
        myRead(0,buf,44);
        ::bzero(buf+44,12);
        
        adler=::vbf_adler32(adler,(const unsigned char*)buf,56);
        
        // now we do regular reads, because we want to read to the end of
        // the file - so partial reads are OK
        if (::lseek(fd,56,SEEK_SET)<0) {
            throw VSystemException("Doing lseek() prior to checksumming in "
                "VBankFileReader::calculateChecksum()");
        }
        
        for (;;) {
            int res=::read(fd,buf,65536);
            if (res<0) {
                throw VSystemException("Doing read() while checksumming in "
                    "VBankFileReader::calculateChecksum()");
            }
            if (res==0) {
                break;
            }
            adler=::vbf_adler32(adler,(const unsigned char*)buf,res);
        }
        
        delete buf;
    } catch (...) {
        delete buf;
        throw;
    }
    
    return adler;
}

void VBankFileReader::verifyChecksum(uword32 expected_checksum) {
    uword32 calculated_checksum=calculateChecksum();
    if (calculated_checksum!=expected_checksum) {
        ostringstream buf;
        buf<<"Expected checksum = "<<expected_checksum<<"; "
           <<"Calculated checksum = "<<calculated_checksum;
        throw VBankFileReaderChecksumInvalidException(buf.str());
    }
}

void VBankFileReader::verifyChecksum() {
    return verifyChecksum(getChecksum());
}

uword64 VBankFileReader::getFileSize() {
    if (stream!=NULL) {
	throw VBankFileReaderStreamedException
	    ("Can't get file size on streamed file");
    }
    off_t size=::lseek(fd,0,SEEK_END);
    if (size<0) {
        throw VSystemException("In VBankFileReader::getFileSize()");
    }
    return (uword64)size;
}

uword64 VBankFileReader::getOriginalBodySize() {
    if (stream!=NULL) {
	throw VBankFileReaderStreamedException
	    ("Can't get original body size on streamed file");
    }
    uword64 orig_pre_footer_size=myReadWord64(48);
    if (orig_pre_footer_size==0) {
	return 0;
    } else {
	return orig_pre_footer_size-56;
    }
}

uword64 VBankFileReader::getBodySize() {
    if (stream!=NULL) {
	throw VBankFileReaderStreamedException
	    ("Can't get body size on streamed file");
    }
    return pre_footer_size-56;
}

uword64 VBankFileReader::getFooterSize() {
    if (stream!=NULL) {
	throw VBankFileReaderStreamedException
	    ("Can't get footer size on streamed file");
    }
    uword64 file_size=getFileSize();
    if (file_size<4+pre_footer_size) {
        return file_size-pre_footer_size;
    }
    
    uword32 num_packets=myReadWord32(pre_footer_size);
    
    if (file_size-pre_footer_size<4+8*num_packets) {
        return file_size-pre_footer_size;
    }
    
    return 4+8*num_packets;
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
