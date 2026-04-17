/*
 * FPIO.h -- Filip Pizlo's own I/O tools, C++ version
 * by Filip Pizlo, 2001, 2002, 2003
 *
 * Purpose: offers a simple API for communicating over a reliable stream (may be half-duplex).
 * Uses a protocol that organizes data into packets of arbitrary length.  Sender does not need to
 * be aware of the ultimate size of the packets he sends; this is taken care of by FPOutput.  Each
 * packet has a size count prepended to it.  Receiver looks at one packet at a time, and does not
 * have to parse the incoming stream into packets; this is taken care of by FPInput.  The act of
 * creating a packet can be undone in FPOutput; likewise, the act of reading a packet can be
 * undone in FPInput.  Both FPInput and FPOutput offer tools to do protocol layering - that is,
 * FPOutput can send formatted data to another FPOutput (so that there are packets within packets).
 * Likewise, FPInput can read in data for parsing from another FPInput.  Both classes also
 * support soft-pipes: FPOutput can send its output to an FPInput.  Likewise, parsed data from
 * within a packet in FPInput can be incorporated into a packet in FPOutput.
 *
 * Both FPOutput and FPInput support endianness conversions.  By default, all data on an FPIO
 * stream is in big-endian.  So, on a little endian machine, there is an ordering flip; on a
 * big endian machine there is no conversion.
 *
 * Both FPOutput and FPInput support the following data types: raw data (providing a length
 * and a character array), C-strings (null-terminated charactter array), and various single
 * word entities (int8_t, word16, word32, word64, float, double).
 *
 * Both FPOutput and FPInput allow for filtering of data before packet creation and after
 * packet parsing (respectively).  This is useful for cases where an encrypted data stream is
 * to be used, but FPIO is still used to keep track of packet boundaries.
 *
 * Each method is marked with the following flags:
 * T - has undergone component testing for all specific scenarios that I could think of in which the
 *     method might malfunction
 * U - has been used in some application, and the application seems to work although has not been
 *     put into production or seen extensive testing
 * E - has been used in several applications, and at least one has seen extensive testing and/or
 *     has been put into production
 * P - this method has been peer-reviewed
 *
 * Methods with no flags or just the U flag should be considered ALPHA.  Methods with T, U, and/or
 * P but without E should be considered BETA.  Methods with T and E should be considered
 * trusted, and should only be modified in extreme circumstances.
 */

#ifndef FPIO_H
#define FPIO_H

#include <fcntl.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <sstream>
#include <typeinfo>

#include "FPIOCommon.h"

class FPInput;
class FPOutput;
class FPIOBuffer;
class FPTargettable;

#include <string>
#include <exception>
#include <sstream>

class FPException: public std::exception {
    private:
        std::string desc,msg,total;
        void makeTotal() {
            std::ostringstream ret;
            ret<<desc.c_str();
            if (msg.length()!=0) {
                ret<<": "<<msg.c_str();
            }
            total=ret.str();
        }
    protected:
        void setDescription(const std::string &_desc) {
            desc=_desc;
            makeTotal();
        }
        void setMessage(const std::string &_msg) {
            msg=_msg;
            makeTotal();
        }
    public:
        FPException() {}
        virtual ~FPException()  noexcept {}
        virtual const char *what() const noexcept { return total.c_str(); }
        virtual const char *getErrorString() { return what(); }
        virtual const char *getDescription() { return desc.c_str(); }
        virtual const char *getMessage() { return msg.c_str(); }
};

class FPSystemException: public FPException {
    private:
        int sys_errno;
    public:
        FPSystemException(int _sys_errno,const std::string &msg=""):
            sys_errno(_sys_errno)
        {
            std::ostringstream buf;
            buf<<"FPIO: SystemError: "<<strerror(sys_errno);
            setDescription(buf.str());
            setMessage(msg);
        }
        int getErrno() { return sys_errno; }
};

class FPBufferFullException: public FPException {
    public:
        FPBufferFullException(const std::string &msg="") {
            setDescription("FPIO: Buffer Full");
            setMessage(msg);
        }
};

class FPTargetFullException: public FPException {
    public:
        FPTargetFullException(const std::string &msg="") {
            setDescription("FPIO: Target Buffer Too Small");
            setMessage(msg);
        }
};

class FPBadFormatException: public FPException {
    public:
        FPBadFormatException(const std::string &msg="") {
            setDescription("FPIO: Bad Format in Stream");
            setMessage(msg);
        }
};

class FPInvalidArgumentException: public FPException {
    public:
        FPInvalidArgumentException(const std::string &msg="") {
            setDescription("FPIO: Invalid Argument");
            setMessage(msg);
        }
};

class FPBadOperationException: public FPException {
    public:
        FPBadOperationException(const std::string &msg="") {
            setDescription("FPIO: Bad Operation");
            setMessage(msg);
        }
};

class FPNotImplementedException: public FPException {
    public:
        FPNotImplementedException(const std::string &msg="") {
            setDescription("FPIO: Not Implemented");
            setMessage(msg);
        }
};

class FPNoPacketException: public FPException {
    public:
        FPNoPacketException(const std::string &msg="") {
            setDescription("FPIO: No Packet");
            setMessage(msg);
        }
};

class FPEOFException: public FPException {
    public:
        FPEOFException(const std::string &msg="") {
            setDescription("FPIO: End Of File (Disconnect)");
            setMessage(msg);
        }
};

class FPPacketTooBigException: public FPException {
    public:
        FPPacketTooBigException(const std::string &msg="") {
            setDescription("FPIO: Packet Too Big");
            setMessage(msg);
        }
};

class FPNotEnoughDataException: public FPException {
    public:
        FPNotEnoughDataException(const std::string &msg="") {
            setDescription("FPIO: Not Enough Data");
            setMessage(msg);
        }
};

class FPThreadDiedException: public FPException {
    public:
        FPThreadDiedException(const std::string &msg="") {
            setDescription("FPIO: Worker Thread Died");
            setMessage(msg);
        }
};

class FPInterruptedException: public FPException {
    public:
        FPInterruptedException(const std::string &msg="") {
            setDescription("FPIO: Got interrupted");
            setMessage(msg);
        }
};

class FPInternalException: public FPException {
    public:
        FPInternalException(const std::string &msg="") {
            setDescription("FPIO: Internal error");
            setMessage(msg);
        }
};

class FPInputBase {
    public:
        virtual ~FPInputBase();

        virtual int getInputEndianness() =0;

        virtual int readInto(FPTargettable *buf)  =0;

        virtual void readBytes(char *data,int len)  =0;
        virtual void readString(char *data,int len)  =0;
        virtual void readWordImpl(char *data,int len)  =0;
        
        virtual const char *requestInputSpan(int len) { return NULL; }

        void readWord(int8_t *x)  { return readWordImpl((char*)x,sizeof(int8_t)); }
        void readWord(uint8_t *x)  { return readWordImpl((char*)x,sizeof(uint8_t)); }
        void readWord(word16 *x)  { return readWordImpl((char*)x,sizeof(word16)); }
        void readWord(uword16 *x)  { return readWordImpl((char*)x,sizeof(uword16)); }
        void readWord(word32 *x)  { return readWordImpl((char*)x,sizeof(word32)); }
        void readWord(uword32 *x)  { return readWordImpl((char*)x,sizeof(uword32)); }
        void readWord(word64 *x)  { return readWordImpl((char*)x,sizeof(word64)); }
        void readWord(uword64 *x)  { return readWordImpl((char*)x,sizeof(uword64)); }
        void readWord(float *x)  { return readWordImpl((char*)x,sizeof(float)); }
        void readWord(double *x)  { return readWordImpl((char*)x,sizeof(double)); }

        int8_t readByte()  { int8_t x; readWord(&x); return x; }
        uint8_t readUByte()  { uint8_t x; readWord(&x); return x; }
        word16 readWord16()  { word16 x; readWord(&x); return x; }
        uword16 readUWord16()  { uword16 x; readWord(&x); return x; }
        word32 readWord32()  { word32 x; readWord(&x); return x; }
        uword32 readUWord32()  { uword32 x; readWord(&x); return x; }
        word64 readWord64()  { word64 x; readWord(&x); return x; }
        uword64 readUWord64()  { uword64 x; readWord(&x); return x; }
        float readFloat()  { float x; readWord(&x); return x; }
        double readDouble()  { double x; readWord(&x); return x; }

        virtual int getBytesAvailable() =0;
        virtual int getAvailableBytes() { return getBytesAvailable(); }
        virtual int getBytesReceived() =0;
};

class FPTargettable {
    public:
        virtual ~FPTargettable();
        virtual int getAvailableForWriting() =0;
        virtual int putBytes(const char *data,int len)  =0;
};

class FPSimpleTargettable:
    public FPTargettable
{
    private:
        char *buffer;
        int size;
    public:
        FPSimpleTargettable():
            buffer(NULL),
            size(0)
        {
        }
        
        virtual ~FPSimpleTargettable() {
            if (buffer!=NULL) {
                free(buffer);
            }
        }
        
        void resetBuffer(bool do_free) {
            if (do_free&&buffer!=NULL) {
                free(buffer);
            }
            buffer=NULL;
            size=0;
        }
        
        char *getBuffer() {
            return buffer;
        }
        
        int getSize() {
            return size;
        }
        
        virtual int getAvailableForWriting() {
            return 0x7FFFFFFF;
        }
        
        virtual int putBytes(const char *data,int len)  {
            if (buffer==NULL) {
                buffer=(char*)malloc(len);
            } else {
                buffer=(char*)realloc(buffer,size+len);
            }
            
            memcpy(buffer+size,data,len);
            size+=len;
            
            return len;
        }
        
        void blitzData(FPTargettable *target,int *len)  {
            *len=target->putBytes(buffer,size);
        }
        
};

class FPOutputBase: public FPTargettable {
    public:
        virtual ~FPOutputBase();

        virtual int getOutputEndianness() =0;

        virtual void writeBytes(const char *data,int len)  =0;
        virtual void writeWordImpl(const char *data,int len)  =0;
        
        virtual char *requestOutputSpan(int len) { return NULL; }

        void writeWord(int8_t x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(uint8_t x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(word16 x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(uword16 x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(word32 x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(uword32 x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(word64 x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(uword64 x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(float x)  { return writeWordImpl((char*)&x,sizeof(x)); }
        void writeWord(double x)  { return writeWordImpl((char*)&x,sizeof(x)); }

        virtual void writeString(const char *str)  =0;
        virtual void writePadding(int len)  =0;

        virtual int getBytesWritten() =0;
        virtual int getBytesToBeSent() =0;

        virtual bool onlyNeedsEstimate() { return false; }
};

class FPInSerializable {
    public:
        virtual ~FPInSerializable() {}
        virtual void readFrom(FPInputBase *in)  =0;
};

class FPOutSerializable {
    public:
        virtual ~FPOutSerializable() {}
        virtual void writeTo(FPOutputBase *out)  =0;
};

// in practice, you should refrain from referring to this class, because
// there may be classes that inherit from InSerializable or OutSerializable
// that have the functionality you need.
class FPSerializable:
    public FPInSerializable,
    public FPOutSerializable
{
};

class FPIOStatistics {
    private:
        uword64 sum_blitz;
        uword64 sum_soak;
        uword64 num_blitz;
        uword64 num_soak;
    public:
        FPIOStatistics();
        FPIOStatistics(const FPIOStatistics &other);
        FPIOStatistics &operator=(const FPIOStatistics &other);
        virtual ~FPIOStatistics();
        void resetStatistics();
        void addBlitz(uword64 amount);
        void addSoak(uword64 amount);
        uword64 getSumBlitz();
        uword64 getSumSoak();
        uword64 getNumBlitz();
        uword64 getNumSoak();
        uword64 getMeanBlitz();
        uword64 getMeanSoak();
};

class FPEndiannessAware {
    private:
        int endianness;
    public:
        FPEndiannessAware();
        virtual ~FPEndiannessAware() {}
        virtual int getEndianness() { return endianness; }
        virtual void setEndianness(int _endianness);
        void setEndianness(FPEndiannessAware *other) {
            setEndianness(other->getEndianness());
        }
};

class FPStatisticsAware {
    private:
        FPIOStatistics *stats;
    protected:
        void addBlitz(uword64 amount);
        void addSoak(uword64 amount);
    public:
        FPStatisticsAware();
        virtual ~FPStatisticsAware() {}
        virtual void setStatistics(FPIOStatistics *_stats) { stats=_stats; }
        void setStatistics(FPStatisticsAware *other) {
            setStatistics(other->getStatistics());
        }
        virtual FPIOStatistics *getStatistics() { return stats; }
};

class FPIOBuffer:
    public FPEndiannessAware,
    public FPStatisticsAware
{
    protected:
        FPIOBuffer();
    public:
        virtual bool isReady() { return true; }
        virtual void resetBuffer();
        virtual void resizeBuffer(int new_size)  ;
};

class FPResizingHack: public FPTargettable {
    private:
        char *buffer;
        int size,first_len,second_len,*len_ptr,cnt;
    public:
        FPResizingHack(char *_buffer,int _size):
            buffer(_buffer),
            size(_size),
            first_len(0),
            second_len(0),
            cnt(0)
        {
            len_ptr=&first_len;
        }
        virtual ~FPResizingHack() {
        }
        void useFirst() { len_ptr=&first_len; }
        void useSecond() { len_ptr=&second_len; }
        int getAvailableForWriting() { return size-cnt; }
        int putBytes(const char *data,int len)  {
            if (cnt+len>size) throw FPBufferFullException();
            memcpy(buffer+cnt,data,len);
            cnt+=len;
            (*len_ptr)+=len;
            return len;
        }
        int getFirst() { return first_len; }
        int getSecond() { return second_len; }
};

class FPSizeEstimate: public FPOutputBase {
    private:
        int size;
    public:
        FPSizeEstimate(): size(0) {}
        virtual ~FPSizeEstimate();
        int getOutputEndianness() { return ENDIANNESS_DONT_CONVERT; }
        void resetBuffer() { size=0; }
        void writeBytes(const char *data,int len)  { size+=len; }
        void writeWordImpl(const char *data,int len)  { size+=len; }
        void writeString(const char *str)  { size+=strlen(str)+1; }
        void writePadding(int len)  { size+=len; }
        int putBytes(const char *data,int len)  { size+=len; return len; }
        int getAvailableForWriting() { return 0x7FFFFFFF; }
        int getBytesWritten() { return size; }
        int getBytesToBeSent() { return size+4; }
        bool onlyNeedsEstimate() { return true; }
};

class FPOutput:
    public FPIOBuffer,
    public FPOutputBase
{
    protected:
        bool
            pre_empty,  // is the blitzable area empty?
            post_empty; // is the writable area empty?
        char *queue;
        int
            puti,       // the place where the next int8_t is put
            geti,       // the place from which we blitz
            length_ptr, // the place where the current packet starts
            size;       // the size of the whole queue
        bool keep;      // delete the queue after we're done?
    public:
        FPOutput(int bytes)  ;  // TUE
        FPOutput(char *_queue,int bytes,bool _keep);
        FPOutput(FPOutput *victim);
        virtual ~FPOutput();    // TUE
        int getOutputEndianness() { return getEndianness(); }
        virtual bool isReady() { return queue!=NULL; }
        virtual void resetBuffer() { pre_empty=true; post_empty=true; puti=0; geti=0; length_ptr=0; }
        virtual void resizeBuffer(int new_size)  ;
        void blitzData(int fd,int *written)  ; // TUE
        void blitzData(FPTargettable *out,int *written)  ; // T
        virtual void beginPacket()  ;  // TUE
        virtual void resetPacket()  ;  // TU
        virtual bool attemptToResetPacket();
        virtual void endPacket()  ;    // TUE
        virtual void endPacketAndCompress()  ;
        virtual int getRequiredForBeginPacket() { return 4; }
        virtual int getPacketOverhead() { return 4; }
        virtual char *requestOutputSpan(int len);
        virtual int putBytes(const char *data,int len)  ;  // works like write() - writes as much as possible
        virtual void writeBytes(const char *data,int len)  ;   // returns -1 if len cannot be written, TUE
        virtual void writeWordImpl(const char *data,int len)  ;    // TUE
        virtual void writeString(const char *str)  ;   // TU
        virtual void writePadding(int len)  ;
        void getFirst(char **data,int *len);    // T
        void getSecond(char **data,int *len);   // T
        bool hasPost() { return !post_empty; }
        int getPostLengthUnsafe() { return post_empty?0:
                                           FPQueue_rollingDiff(puti,length_ptr,size)-4; }
        virtual int getPostLength() { return getPostLengthUnsafe(); }
        bool hasPre() { return !pre_empty; }
        int getPreLengthUnsafe() { return pre_empty?0:
                FPQueue_rollingDiff(length_ptr,geti,size); }
        virtual int getPreLength() { return getPreLengthUnsafe(); }
        int getSize() { return size; }
        int getLargestPossiblePacket() { return getSize()-getPacketOverhead(); }
        int getBytesWritten() { return getPostLength(); }
        int getBytesToBeSent() { return getBytesWritten()+4; }
        int getBytesBuffered() { return getPreLength(); }
        int getAvailableForWritingUnsafe() {
            return getSize()-getPreLengthUnsafe()-(post_empty?0:
                        FPQueue_rollingDiff(puti,length_ptr,size));
        }
        virtual int getAvailableForWriting() { return getAvailableForWritingUnsafe(); }
};

class FPThreadBlitzCounter {
    private:
        int cnt;
    public:
        FPThreadBlitzCounter(): cnt(0) {}
        virtual ~FPThreadBlitzCounter() {}
        virtual void incrementCounter(int x) { cnt+=x; }
        virtual int getCount() { return cnt; }
};

#define FP_THR_MODE_SECTION_ORIENTED 0
#define FP_THR_MODE_DOUBLE_LOCKING 1
#define FP_THR_MODE_FULL_BLOCKING 2

class FPThreadContext;
class FPThreadOutputManager {
    protected:
        FPThreadContext *context;
    public:
        FPThreadOutputManager(int sock,
                              FPOutput *out,
                              FPThreadBlitzCounter *cnt,
                              int mode)  ;
        virtual ~FPThreadOutputManager();
        void setBlitzTimeout(uword64 _blitz_timeout_us);
        void setMinToBlitz(long _min_to_blitz);
        uword64 getBlitzTimeout();
        long getMinToBlitz();
        FPOutput *getBuffer();
        void waitForCompletion()  ;
};

class FPAutoOutput: public FPOutput {
    private:
        int fd;
        uword64 blitz_count;
    public:
        FPAutoOutput(int bytes)  ;
        FPAutoOutput(int _fd,int bytes)  ;
        virtual ~FPAutoOutput();
        void setFileDescriptor(int _fd) { fd=_fd; }
        virtual void beginPacket()  ;
        virtual int putBytes(const char *data,int len)  ;
        virtual void writeBytes(const char *data,int len)  ;
        virtual void writeWordImpl(const char *data,int len)  ;
        virtual void writeString(const char *data)  ;
        virtual void writePadding(int len)  ;
        void flushData()  ;
        uword64 getBlitzCount();
};

class FPInput:
    public FPIOBuffer,
    public FPInputBase,
    public FPTargettable
{
    friend class FPOutput;
    protected:
        
        bool
            pre_empty,   // is the readable area empty?
            post_empty,   // is the soakable area empty?
            in_packet;
        
        char *queue;
        
        int
            puti,         // the place where the next int8_t is put
            geti,         // the place from which we read
            upto,         // the beginning of the next packet
            size;         // the size of the whole queue
        
    public:
        FPInput(int bytes)  ;   // TUE
        virtual ~FPInput(); // TUE
        int getInputEndianness() { return getEndianness(); }
        virtual bool isReady() { return queue!=NULL; }
        virtual void resetBuffer() { pre_empty=1; post_empty=1; puti=0; geti=0; upto=0; }
        virtual void resizeBuffer(int new_size)  ;
        int putBytes(const char *data,int len)  ;  // used by FPOutput::blitz, T
        void soakData(int fd,int *read)  ; // TUE
        virtual void soakNewPacket(int fd,int *read)  ;
        void beginPacket()  ;  // TUE
        void beginPacketAndDecompress()  ;
        bool endPacket()  ;    // TUE
        bool hasPacket()  ;    // TU
        int readInto(FPTargettable *buf)  ;
        void readBytes(char *data,int len)  ;  // TUE
        void readString(char *data,int len)  ; // TUE
        void readWordImpl(char *data,int len)  ;   // TUE
        const char *requestInputSpan(int len);
        void getFirst(char **data,int *len);    // T
        void getSecond(char **data,int *len);   // T
        int getSize() { return size; }
        bool hasPost() { return !post_empty; }
        bool hasPre() { return !pre_empty; }
        int getPostLength() { return post_empty?0:
                FPQueue_rollingDiff(puti,upto,size); }
        int getPreLength() { return pre_empty?0:    // TUE
                FPQueue_rollingDiff(upto,geti,size); }
        int getAvailableForWriting() { return size-getPreLength()-getPostLength(); }    // TUE
        int getBytesAvailable() { return getPreLength(); }
        int getBytesBuffered() { return getPostLength(); }
        int getBytesReceived() { return getBytesAvailable()+4; }
};

class FPInterruptableInput:
    public FPInput
{
    protected:
        
        int my_pipe[2];     // interrupt pipe
        
    public:
        
        FPInterruptableInput(int bytes)  ;
        
        virtual ~FPInterruptableInput();
        
        void soakNewPacket(int fd,int *read)  ;
        
        void interruptSoakNewPacket()  ;
        
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
