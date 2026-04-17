/*
 * VBankFileReader.h -- reads a bank file in a random-access fashion.  also
 *                      contains utility methods for re-generating the index,
 *                      verifying the checksum, and re-generating the
 *                      checksum.
 * by Filip Pizlo, 2004
 */

#ifndef V_BANK_FILE_READER_H
#define V_BANK_FILE_READER_H

#include "VException.h"
#include "VPacket.h"
#include "VBankBuilder.h"
#include "VBankName.h"
#include "VBankFileReaderBase.h"
#include <map>

class VBankFileReaderException: public VException {};

class VBankFileReaderStreamedException: public VBankFileReaderException {
 public:
    VBankFileReaderStreamedException(const std::string &msg="") {
	setStrings("The VBF file was opened using a stream reader, so "
		   "the requested operation won't work",msg);
    }
};

class VBankFileReaderReadOnlyException: public VBankFileReaderException {
 public:
    VBankFileReaderReadOnlyException(const std::string &msg="") {
	setStrings("The VBF file was opened read-only but the requested "
		   "action requires write capability",msg);
    }
};

class VBankFileReaderNoIndexException: public VBankFileReaderException {
 public:
    VBankFileReaderNoIndexException(const std::string &msg="") {
	setStrings("The VBF file does not have an index, yet the "
		   "requested operation requires that an index be "
		   "available",msg);
    }
};

class VBankFileReaderIndexOutOfBoundsException:
public VBankFileReaderException {
 public:
    VBankFileReaderIndexOutOfBoundsException(const std::string &msg="") {
	setStrings("The requested bank index is outside the bounds "
		   "of the given VBF file",msg);
    }
};

class VBankFileReaderNoChecksumException: public VBankFileReaderException {
 public:
    VBankFileReaderNoChecksumException(const std::string &msg="") {
	setStrings("The VBF file does not have a checksum, yet the "
		   "requested operation requires that a checksum be "
		   "available",msg);
    }
};

class VBankFileReaderChecksumInvalidException:
public VBankFileReaderException {
 public:
    VBankFileReaderChecksumInvalidException(const std::string &msg="") {
	setStrings("VBF checksum verification indicates that the given "
		   "file is corrupt",msg);
    }
};

class VBankFileReaderBadFormatException: public VBankFileReaderException {
 public:
    VBankFileReaderBadFormatException(const std::string &msg="") {
	setStrings("The VBF file is badly formatted",msg);
    }
};

class VBankFileReaderBadMagicNumberException:
public VBankFileReaderBadFormatException {
 public:
    VBankFileReaderBadMagicNumberException(const std::string &msg="") {
	setStrings("The magic number in the given VBF file is incorrect; "
		   "this error implies that the given file is not a VBF "
		   "file and should be read using a different library",
		   msg);
    }
};

class VBankFileReaderBadVersionNumberException:
public VBankFileReaderBadFormatException {
 public:
    VBankFileReaderBadVersionNumberException(const std::string &msg="") {
	setStrings("The version number in the VBF file is not recognized; "
		   "this error implies that you should probably upgrade "
		   "your VBF library",
		   msg);
    }
};

class VBankFileStreamReader;

class VBankFileReader: public VBankFileReaderBase {
 private:
        
    int fd;
        
    bool read_only;
        
    VBankFileStreamReader *stream;
    VPacket *next_packet;

    // this is the offset of the next packet to read.
    // that is, it is the offset of the packet that
    // follows the packet we read previously.
    word64 packet_offset;

    // this is the index of the next packet to read.
    // that is, it is the index of the packet that
    // follows the packet we read previously.
    uword32 packet_index;   
        
    // the base page of the index; only used for mumnapping
    char *index_base;

    // we mmap the index, and here's the pointer to the
    // first page of the index; may be NULL, indicating
    // that we have no index        
    char *index;

    // if we have an index, this is the number of
    // packets in the file.  otherwise the value here
    // is undefined        
    uword32 num_packets;
        
    // the size of the file up to the footer.
    // this is not necessarily equal to the
    // pre_footer_size field in the file.  if
    // the footer is missing, this field will
    // just be exactly equal to the size of
    // the file
    uword64 pre_footer_size;
        
    typedef std::map< VBankName, VBankBuilder* >
	bank_builder_map;
    typedef std::vector< VBankBuilder* >
	bank_builder_vec;
        
    bank_builder_map bank_builders;
    bank_builder_vec final_bank_builders;
        
    void myRead(word64 offset,char *buf,uword32 len);
    uint8_t myReadByte(word64 offset);
    uword32 myReadWord32(word64 offset);
    uword64 myReadWord64(word64 offset);
        
    uword32 myWrite(word64 offset,
		    uword32 adler,
		    const char *buf,
		    uword32 len);
    uword32 myWrite(word64 offset,
		    uword32 adler,
		    uword32 value);
    uword32 myWrite(word64 offset,
		    uword32 adler,
		    uword64 value);

    void myWrite(word64 offset,const char *buf,uword32 len);
    void myWrite(word64 offset,uword32 value);
    void myWrite(word64 offset,uword64 value);

    void skipNextPacket();
    void streamSkipTo(uword32 index);
        
    void unmapIndex();
        
 public:
        
    // NOTE ABOUT EXCEPTIONS: some methods' documentation will say
    // 'all exceptions are fatal'.  Such a statement indicates that you
    // are given no guarantees about the state of this object after getting
    // an exception from such a method.  In essence, the only thing you can
    // do after getting an exception from such a method is to delete this
    // object.  (Well, since this is a reference counted object, you wouldn't
    // actually delete it - you would instead rid yourself of your reference
    // to it.)
        
    // construct a reader by openning the given file.  if map_index is true,
    // the index is mapped.  if it cannot be mapped (either because of an
    // error or because it is simply not present), the file is not openned,
    // the reader is not created, and you get an exception.  if you wish
    // to be able to read the file even without an index, you should pass
    // false to map_index.  You can at any time attempt to map the index by
    // calling mapIndex().  if mapIndex() fails, you can still use this
    // reader in a no-index mode.
    VBankFileReader(const std::string &filename,
		    bool map_index=true,
		    bool read_only=true);
        
    virtual ~VBankFileReader();
        
    // get the file's run number
    long getRunNumber() const noexcept {
	return run_number;
    }
        
    // get the file's configuration mask
    const std::vector< bool > &getConfigMask() const noexcept {
	return config_mask;
    }
        
    // add a bank builder.  all exceptions fatal.
    // note that the array event and simulation data bank builders are
    // added automatically by the VBankFileReader constructor.  you only
    // have to use this method if you wish to read your own custom banks.
    void putBankBuilder(const VBankName &name,
			VBankBuilder *bank_builder);

    // remove all bank builders, including the defaults.
    void resetBankBuilders();

    // add a final bank builder.
    void addFinalBankBuilder(VBankBuilder *bank_builer);

    // query for a bank builder; if it's not found, you get NULL.  all
    // exceptions fatal.
    VBankBuilder *getBankBuilder(const VBankName &name);
        
    // returns true if the file is being streamed.  this is the equivalent
    // of read-only, except that you cannot compute checksums or get the
    // file/body/footer size.  a file is streamed if it is a bzip2 file.
    bool isStreamed() const noexcept {
	return stream!=NULL;
    }
	
    // returns true if we have an index; this will always be true if
    // allow_no_index (see constructor above) was false.  if false, this
    // means that: numPackets() will always throw the no index
    // exception, and hasPacket()/readPacket() can only be called with
    // monotonically increasing values of index.
    bool hasIndex() const noexcept {
	return index!=NULL;
    }
        
    // maps the index.  unmaps it if it was already mapped.  returns true
    // if the index was mapped successfully.  returns false if there was
    // no index.  throws an exception if there was an error in the format
    // of the index.
    bool mapIndex();

    // returns true if we have a checksum
    bool hasChecksum();
        
    // returns the total number of packets in the whole file.  if there
    // is no index, will throw the no index exception.
    uword32 numPackets() const;
        
    // checks if the packet with the given index is available.  if there
    // is an index, this is basically just a index<numPackets() check.
    // otherwise, this method will read up to but not including the
    // packet with the given index and then verify that we have not yet
    // hit the pre_footer_size.  if not, it will return true.  if so,
    // it will return false.  note that if there is no index, the index
    // must be greater than any index used previously.  all
    // exceptions fatal if there is no index; otherwise exceptions are not
    // fatal.
    bool hasPacket(uword32 index);
        
    // read the packet with the given index.  if there is an index
    // available, this method turns into a simple random-access operation.
    // if there is no index available, this method must first skip over
    // all of the packets not yet read that have a smaller index.  note
    // that if there is no index, the index must be greater than
    // any index used previously.  all exceptions fatal if there is no
    // index; otherwise exceptions are not fatal.
    VPacket *readPacket(uword32 index);
        
    // reset sequential read.  if there is an index, this has no noticable
    // effect other than perhaps one of performance.  if there is no index,
    // this allows the user to start reading the file from the beginning
    // again.  if you have an index but are doing sequential reads, calling
    // this method may give you a slight performance improvement if the
    // next read you're planning on doing is on index 0.  all exceptions 
    // fatal
    void resetSequentialRead();
        
    // regenerate the index and the checksum, even if both were already
    // present.  this may take some time.  after the index and checksum
    // are generated, both are written to the file and also made available
    // to this object.  all exceptions fatal.  note that to call this,
    // you must have passed false to read_only in the constructor.
    uword32 generateIndexAndChecksum();
        
    // get the checksum that is stored in the file.  if hasChecksum()
    // is false, this will throw an exception.  all exceptions fatal.
    uword32 getChecksum();
        
    // calculate the checksum, and return it.  if the value returned
    // by this method is different than the file returned by
    // getChecksum(), then the file must be corrupt.  note that the
    // file might still be corrupt if the value returned here is
    // the same as the value from getChecksum().
    uword32 calculateChecksum();
        
    // verify the given checksum.  will calculate the checksum over the
    // file and then verify that it is equal to the given checksum.  if
    // the calculated checksum is not the same as the expected one, this
    // method will throw an exception.  all exceptions other than
    // VBankFileReaderChecksumInvalidException are fatal.
    void verifyChecksum(uword32 expected_checksum);
        
    // verify the checksum that is stored in the file.  equivalent to
    // calling verifyChecksum(getChecksum()).  all exceptions other than
    // VBankFileReaderChecksumInvalidException are fatal.
    void verifyChecksum();
        
    // get the size of the whole file.  note that this may not be
    // equal to getHeaderSize()+getBodySize()+getFooterSize(), since
    // if any amount of data is appended to an otherwise perfectly
    // valid VBF file (that has a footer), the reader will simply
    // ignore this data.
    uword64 getFileSize();
        
    // get the size of the header.  this is always the same.
    static uword64 getHeaderSize()  noexcept { return 56; }
    
    // tells you how big the body is according the header.
    uword64 getOriginalBodySize();
    
    // get the size of the body.  this is the part that contains all
    // those packets.
    uword64 getBodySize();
        
    // get the size of the footer.  the footer contains the index.  the
    // principle guiding this method is that if the footer is not
    // truncated, then the footer's expected size is returned (in
    // otherwords, you get 4+num_packets*8).  if the footer is truncated,
    // you get its effective size (in otherwords, you get
    // getFileSize()-getBodySize()-getHeaderSize()).  note that this
    // method does not require mapIndex() to have been called, so the
    // num_packets value that I refer to in the expression above is what
    // is read from the file, not what is returned from numPackets().
    uword64 getFooterSize();
        
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
