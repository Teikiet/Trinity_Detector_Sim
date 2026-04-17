/*
 * FPIO.cpp -- Filip Pizlo's own I/O tools
 * by Filip Pizlo, 2001, 2002, 2003
 */

#include "FPIO.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#define DEFAULT_ENDIAN LITTLE_ENDIAN

FPInputBase::~FPInputBase() {
  // nothing to see here
}

FPTargettable::~FPTargettable(){
  // nothing to see here
}

FPOutputBase::~FPOutputBase(){
  // nothing to see here
}

FPSizeEstimate::~FPSizeEstimate(){
  // nothing to see here
}

FPEndiannessAware::FPEndiannessAware() {
#if (BYTE_ORDER==DEFAULT_ENDIAN)
    //printf("little endian\n");
    endianness=ENDIANNESS_DONT_CONVERT;
#else
    //printf("big endian\n");
    endianness=ENDIANNESS_CONVERT;
#endif
}

FPStatisticsAware::FPStatisticsAware():
    stats(NULL)
{
}

FPIOBuffer::FPIOBuffer()
{
    //printf("endianness=%d\n",endianness);
}

void FPEndiannessAware::setEndianness(int _endianness) {
#if (BYTE_ORDER==LITTLE_ENDIAN)
    switch (_endianness) {
        case ENDIANNESS_BIG_ENDIAN: endianness=ENDIANNESS_CONVERT; break;
        case ENDIANNESS_LITTLE_ENDIAN: endianness=ENDIANNESS_DONT_CONVERT; break;
        default: endianness=_endianness; break;
    }
#else
    switch (_endianness) {
        case ENDIANNESS_BIG_ENDIAN: endianness=ENDIANNESS_DONT_CONVERT; break;
        case ENDIANNESS_LITTLE_ENDIAN: endianness=ENDIANNESS_CONVERT; break;
        default: endianness=_endianness; break;
    }
#endif
}

void FPIOBuffer::resetBuffer() {
}

void FPIOBuffer::resizeBuffer(int new_size)  {
}

FPOutput::FPOutput(int bytes)  :
    pre_empty(1),
    post_empty(1),
    queue((char*)new char[bytes]),
    puti(0),
    geti(0),
    length_ptr(0),
    size(bytes),
    keep(true)
{
}

FPOutput::FPOutput(FPOutput *victim):
    pre_empty(victim->pre_empty),
    post_empty(victim->post_empty),
    queue(victim->queue),
    puti(victim->puti),
    geti(victim->geti),
    length_ptr(victim->length_ptr),
    size(victim->size),
    keep(victim->keep)
{
    victim->queue=NULL;
    setStatistics(victim->getStatistics());
    delete victim;
}

FPOutput::FPOutput(char *_queue,int bytes,bool _keep):
    pre_empty(1),
    post_empty(1),
    queue(_queue),
    puti(0),
    geti(0),
    length_ptr(0),
    size(bytes),
    keep(_keep)
{
}

FPOutput::~FPOutput() {
    if (queue!=NULL&&keep) delete[] queue;
}

void FPOutput::resizeBuffer(int new_size)  {
    int len;
    if (new_size<size) {
        throw FPInvalidArgumentException("In FPOutput::resizeBuffer()");
    }
    if (new_size==size) return;
    char *new_buf=new char[new_size];
    FPResizingHack hack(new_buf,new_size);
    blitzData(&hack,&len);
    if (hasPost()) {
        endPacket();
        hack.useSecond();
        blitzData(&hack,&len);
    }
    delete queue;
    queue=new_buf;
    geti=0;
    size=new_size;
    if (hack.getFirst()) {
        length_ptr=hack.getFirst();
        pre_empty=false;
    } else {
        length_ptr=0;
        pre_empty=true;
    }
    if (hack.getSecond()) {
        puti=hack.getSecond()+length_ptr;
        post_empty=false;
    } else {
        puti=length_ptr;
        post_empty=true;
    }
}

void FPOutput::blitzData(int fd,int *written)  {
    int res,n;
    // reset the write count
    *written=0;
    // if it's empty, return (so if res==0&&written==0 then we shouldn't
    // have called the fucker)
    if (pre_empty) return;
    // if geti is greater than length_ptr, we should first write from geti
    // to the end of the buffer
    if (geti>=length_ptr) {
        n=size-geti;
        for (;;) {
            res=write(fd,queue+geti,n);
            if (res>=0||errno!=EINTR) break;
        }
        // if res<0 then we had a write error - exit right now
        // (so if written==0&&res<0 then the filedescriptor was not ready for
        // writing)
        if (res<0) {
            throw FPSystemException(errno,"In FPOutput::blitzData()");
        }
        // set the write count
        *written=res;
        // if res<n then we didn't write all that we should have - this
        // means that the filedescriptor was not ready for the full load we
        // had prepared.  that's ok, though.
        if (res<n) {
            geti+=res;
            addBlitz(*written);
            return;
        }
        // set geti to zero, since we obviously wrote to the end of the
        // buffer
        geti=0;
        // if length_ptr is also zero, then we're done writing, so set
        // pre_empty and return
        if (!length_ptr) {
            pre_empty=true;
            addBlitz(*written);
            return;
        }
    }
    // if geti is smaller than length_ptr, we should write the difference
    if (geti<length_ptr) {
        n=length_ptr-geti;
        for (;;) {
            res=write(fd,queue+geti,n);
            if (res>=0||errno!=EINTR) break;
        }
        // again, we had ourselves a write error
        if (res<0) {
            addBlitz(*written);
            throw FPSystemException(errno,"In FPOutput::blitzData()");
        }
        // increment the counts by res
        geti+=res;
        *written+=res;
        // if we wrote all that we could have, that means that we emptied
        // the buffer
        if (res==n) pre_empty=true;
    }
    addBlitz(*written);
}

void FPOutput::blitzData(FPTargettable *out,int *written)  {
    int res,n;
    // reset the write count
    *written=0;
    // if it's empty, return (so if res==0&&written==0 then we shouldn't
    // have called the fucker)
    if (pre_empty) return;
    // if geti is greater than length_ptr, we should first write from geti
    // to the end of the buffer
    if (geti>=length_ptr) {
        n=size-geti;
        res=out->putBytes(queue+geti,n);
        // comment this out... there is no way for n to change in call to putBytes
        //n=size-geti;
        // set the write count
        *written=res;
        if (res<n) {
            addBlitz(*written);
            geti+=res;
            return;
        }
        // set geti to zero, since we obviously wrote to the end of the
        // buffer
        geti=0;
        // if length_ptr is also zero, then we're done writing, so set
        // pre_empty and return
        if (!length_ptr) {
            addBlitz(*written);
            pre_empty=true;
            return;
        }
    }
    // if geti is smaller than length_ptr, we should write the difference
    if (geti<length_ptr) {
        n=length_ptr-geti;
        try {
            res=out->putBytes(queue+geti,n);
        } catch (const FPException &e) {
            // again, we had ourselves a write error
            addBlitz(*written);
            throw;
        }
        // re-evaluate n (n may have changed in the call to putBytes)
        n=length_ptr-geti;
        // increment the counts by res
        geti+=res;
        *written+=res;
        // we wrote all that we could have, that means that we emptied
        // the buffer
        if (res==n) pre_empty=true;
    }
    addBlitz(*written);
}

void FPOutput::beginPacket()  {
    if (!post_empty) {
        throw FPBadOperationException("In FPOutput::beginPacket()");
    }
    if (!pre_empty&&(puti==geti||FPQueue_rollingDiff(geti,puti,size)<4)) {
        throw FPBufferFullException("In FPOutput::beginPacket()");
    }
    post_empty=false;   // say that we have a packet
    puti+=4;        // that's how long the length is!
    if (puti>=size) {
        puti-=size; // recycle
    }
}

void FPOutput::resetPacket()  {
    if (post_empty) {
        throw FPNoPacketException("In FPOutput::resetPacket()");
    }
    puti=length_ptr;    // set the puti back to the length_ptr
    post_empty=true;        // set the thing to be empty
}

bool FPOutput::attemptToResetPacket() {
    if (!post_empty) {
        puti=length_ptr;
        post_empty=true;
        return true;
    }
    return false;
}

void FPOutput::endPacket()  {
    if (post_empty) {
        throw FPNoPacketException();
    }
    int i,j,n=FPQueue_rollingDiff(puti,length_ptr,size)-4;
    char *ptr=(char*)&n;    // access the length using bytes
    if (getEndianness()==ENDIANNESS_CONVERT) {
        for (i=length_ptr,j=3;j>=0&&i<size;i++,j--) {
            queue[i]=ptr[j];    // write it in referese order
        }
        for (i=0;j>=0;i++,j--) {
            queue[i]=ptr[j];    // finish on the flipside
        }
    } else {
        for (i=length_ptr,j=0;j<4&&i<size;i++,j++) {
            queue[i]=ptr[j];    // write it in order
        }
        for (i=0;j<4;i++,j++) {
            queue[i]=ptr[j];    // finish on the flipside
        }
    }
    post_empty=true;    // set the packet to be empty
    length_ptr=puti;    // set the length_ptr to where we are writing
    pre_empty=false;
}

void FPOutput::endPacketAndCompress()  {
    /*if (post_empty) return -1;
    int i,j,n=FPQueue_rollingDiff(puti,length_ptr,size)-4;*/
    // ah, shit, I'm drunk, finish this later
    // think about using zlib and then think about using
    // bzip2
    throw FPNotImplementedException("FPOutput::endPacketAndCompress()");
}

void FPOutput::writeString(const char *string)  {
    if (post_empty) {
        throw FPNoPacketException("FPOutput::writeString()");
    }
    int n,i;
    if (puti>=geti) {   // queue is in flip-mode
        if (puti==geti&&!post_empty) {
            throw FPBufferFullException("In FPOutput::writeString()");
        }
        n=size-puti;
        // write to mainside
        for (i=0;i<n;i++) {
            queue[puti+i]=string[i];
            if (!string[i]) {
                puti+=i+1;
                if (puti==size) puti=0;
                return;
            }
        }
        // write to flipside
        for (i=0;i<geti;i++) {
            queue[i]=string[n+i];
            if (!string[n+i]) {
                puti=i+1;
                return;
            }
        }
    } else { // queue is in main-mode
        // write...
        for (i=0;i<geti-puti;i++) {
            queue[puti+i]=string[i];
            if (!string[i]) {
                puti+=i+1;
                return;
            }
        }
    }
    // we never reached the end of the string, so the string is too large for
    // the buffer.  return error.
    throw FPBufferFullException("In FPOutput::writeString()");
}

char *FPOutput::requestOutputSpan(int len) {
    if (post_empty) {
        return NULL;
    }
    
    if (puti==geti) {
        return NULL;
    }
    
    if (puti>geti) {
        if (len>size-puti) {
            return NULL;
        }
        
        char *ret=queue+puti;
        puti+=len;
        if (puti==size) puti=0;
        
        return ret;
    }
    
    if (len>geti-puti) {
        return NULL;
    }
    
    char *ret=queue+puti;
    puti+=len;
    
    return ret;
}

void FPOutput::writeBytes(const char *data,int len)  {
    if (post_empty) {
        throw FPNoPacketException("In FPOutput::writeBytes()");
    }
    int n;
    if (puti>=geti) {   // queue is in flip-mode
        if (puti==geti&&!post_empty) {
            std::ostringstream buf;
            buf<<"In FPOutput::writeBytes(): "
               <<"puti = geti = "<<puti
               <<" and post_empty = false";
            throw FPBufferFullException(buf.str());
        }
        n=size-puti;    // determine how much more we have on mainside
        if (len>n) {    // not enough
            if (len>n+geti) {
                std::ostringstream buf;
                buf<<"In FPOutput::writeBytes: "
                   <<"(len="<<len
                   <<") > (n="<<n
                   <<") + (geti="<<geti
                   <<")";
                throw FPBufferFullException(buf.str());
            }
            memcpy(queue+puti,data,n);  // copy to mainside
            memcpy(queue,data+n,len-n); // copy to flipside
            puti=len-n; // set puti to the correct place
        } else {    // enough
            memcpy(queue+puti,data,len);    // copy to mainside
            puti+=len;  // increment
            if (puti==size) puti=0; // flip if necessary
        }
    } else {    // queue is in main-mode
        if (len>geti-puti) {
            std::ostringstream buf;
            buf<<"In FPOutput::writeBytes(): (len="<<len
               <<") > (geti="<<geti
               <<") - (puti="<<puti
               <<")";
            throw FPBufferFullException(buf.str());
        }
        memcpy(queue+puti,data,len);    // copy
        puti+=len;  // increment
    }
}

void FPOutput::writePadding(int len)  {
    // duplicate of writeBytes() except that no memcpy() is done
    if (post_empty) {
        throw FPNoPacketException("In FPOutput::writePadding()");
    }
    int n;
    if (puti>=geti) {   // queue is in flip-mode
        if (puti==geti&&!post_empty) {
            throw FPBufferFullException("In FPOutput::writePadding()");
        }
        n=size-puti;    // determine how much more we have on mainside
        if (len>n) {    // not enough
            if (len>n+geti) {
                throw FPBufferFullException("In FPOutput::writePadding()");
            }
            puti=len-n; // set puti to the correct place
        } else {    // enough
            puti+=len;  // increment
            if (puti==size) puti=0; // flip if necessary
        }
    } else {    // queue is in main-mode
        if (len>geti-puti) {
            throw FPBufferFullException("In FPOutput::writePadding()");
        }
        puti+=len;  // increment
    }
}

int FPOutput::putBytes(const char *data,int len)  {
    if (post_empty) {
        throw FPNoPacketException("In FPOutput::putBytes()");
    }
    int n,ret=0,toWrite;
    if (puti>=geti) {   // queue is in flip-mode
        if (puti==geti&&!post_empty) {
            throw FPBufferFullException("In FPOutput::putBytes()");
        }
        n=size-puti;    // determine how much more we have on mainside
        if (len>n) {    // not enough
            //if (len>n+geti) return -1;    // not enough total!
            toWrite=FP_min(len-n,geti);
            memcpy(queue+puti,data,n);  // copy to mainside
            memcpy(queue,data+n,toWrite);   // copy to flipside
            puti=toWrite;   // set puti to the correct place
            ret=n+toWrite;
        } else {    // enough
            memcpy(queue+puti,data,len);    // copy to mainside
            puti+=len;  // increment
            ret=len;
            if (puti==size) puti=0; // flip if necessary
        }
    } else {    // queue is in main-mode
        //if (len>geti-puti) return -1; // not enough room!
        toWrite=FP_min(len,geti-puti);
        memcpy(queue+puti,data,toWrite);    // copy
        puti+=toWrite;  // increment
        ret=toWrite;
    }
    return ret;
}

void FPOutput::writeWordImpl(const char *data,int len)  {
    // puts everything in correct endianness.
    if (getEndianness()==ENDIANNESS_CONVERT) {
        if (post_empty) {
            throw FPNoPacketException("In FPOutput::writeWordImpl()");
        }
        int n,i,j;
        if (puti>=geti) {   // queue is in flip-mode
            if (puti==geti&&!post_empty) {
                throw FPBufferFullException("In FPOutput::writeWordImpl()");
            }
            n=size-puti;    // determine how much more we have on mainside
            if (len>n) {    // not enough
                if (len>n+geti) {
                    throw FPBufferFullException("In FPOutput::writeWordImpl()");
                }
                // copy to mainside
                for (i=puti,j=len-1;i<size;i++,j--) {
                    queue[i]=data[j];
                }
                // copy to flipside
                for (i=0;j>=0;i++,j--) {
                    queue[i]=data[j];
                }
                puti=len-n; // set puti to the correct place
            } else {    // enough
                // copy to mainside
                for (i=puti,j=len-1;j>=0;i++,j--) {
                    queue[i]=data[j];
                }
                puti+=len;  // increment
                if (puti==size) puti=0; // flip if necessary
            }
        } else {    // queue is in main-mode
            if (len>geti-puti) {
                throw FPBufferFullException("In FPOutput::writeWordImpl()");
            }
            // copy
            for (i=puti,j=len-1;j>=0;i++,j--) {
                queue[i]=data[j];
            }
            puti+=len;  // increment
        }
    } else {
        FPOutput::writeBytes(data,len);
    }
}

void FPOutput::getFirst(char **data,int *len) {
    int i=length_ptr+4;
    if (i>=size) i-=size;
    *data=queue+i;
    if (puti>=i) *len=puti-i;
    else *len=size-i;
}

void FPOutput::getSecond(char **data,int *len) {
    int i=length_ptr+4;
    if (i>=size) i-=size;
    if (puti>=i) {
        *data=NULL;
        *len=0;
    } else {
        *data=queue;
        *len=puti;
    }
}

FPInput::FPInput(int bytes)  :
    pre_empty(true),
    post_empty(true),
    in_packet(false),
    queue((char*)new char[bytes]),
    puti(0),
    geti(0),
    upto(0),
    size(bytes)
{
}

FPInput::~FPInput() {
    if (queue!=NULL) delete[] queue;
}

void FPInput::resizeBuffer(int new_size)  {
    if (new_size<size) {
        throw FPInvalidArgumentException("In FPInput::resizeBuffer()");
    }
    if (new_size==size) return;
    char *new_buf=new char[new_size];
    FPResizingHack hack(new_buf,new_size);
    // only do this if there is a packet
    if (hasPre()) {
        readInto(&hack);
    }
    hack.useSecond();
    if (hasPost()) {
        if (upto<puti) {
            hack.putBytes(queue+upto,puti-upto);
        } else {
            hack.putBytes(queue+upto,size-upto);
            hack.putBytes(queue,puti);
        }
    }
    delete queue;
    queue=new_buf;
    size=new_size;
    geti=0;
    if (hack.getFirst()) {
        upto=hack.getFirst();
        pre_empty=false;
    } else {
        upto=0;
        pre_empty=true;
    }
    if (hack.getSecond()) {
        puti=hack.getSecond()+upto;
        post_empty=false;
    } else {
        puti=upto;
        post_empty=true;
    }
}

int FPInput::putBytes(const char *data,int len)  {
    ///*DEBUG*/printf("len=%d\n",len);
    int n,avail,ret=0,toWrite;
    avail=getAvailableForWriting();
    if (!avail) {
        throw FPBufferFullException("In FPInput::putBytes()");
    }
    if (puti>=geti) {
        if (!len) return 0;
        n=size-puti;
        toWrite=FP_min(n,len);
        memcpy(queue+puti,data,toWrite);
        ret+=toWrite;
        len-=toWrite;
        if (toWrite>0) post_empty=false;
        if (toWrite<n) {
            puti+=toWrite;
            addSoak(ret);
            ///*DEBUG*/printf("ret=%d\n",ret);
            return ret;
        }
        puti=0;
    }
    if (!len) {
        ///*DEBUG*/printf("ret=%d\n",ret);
        addSoak(ret);
        return ret;
    }
    n=geti-puti;
    toWrite=FP_min(n,len);
    memcpy(queue+puti,data+ret,toWrite);
    puti+=toWrite;
    ret+=toWrite;
    post_empty=false;
    ///*DEBUG*/printf("ret=%d\n",ret);
    addSoak(ret);
    return ret;
}

int FPInput::readInto(FPTargettable *buf)  {
    int n,res,ret=0;
    if (pre_empty) {
        throw FPNoPacketException("In FPInput::readInto()");
    }
    if (geti>=upto) {
        n=size-geti;
        if (buf->getAvailableForWriting()==0) {
            return ret;
        }
        res=buf->putBytes(queue+geti,n);
        ret+=res;
        if (res<n) {
            geti+=res;
            return ret;
        }
        geti=0;
        if (geti==upto) {
            pre_empty=true;
            return ret;
        }
    }
    n=upto-geti;
    if (buf->getAvailableForWriting()==0) {
        return ret;
    }
    res=buf->putBytes(queue+geti,n);
    ret+=res;
    geti+=res;
    if (geti==upto) pre_empty=true;
    return ret;
}

void FPInput::soakNewPacket(int fd,int *read)  {
    int nread,throwaway;
    if (read==NULL) read=&throwaway;
    *read=0;
    while (!hasPacket()) {
        nread=getAvailableForWriting();
        soakData(fd,&nread);
        *read+=nread;
    }
}

void FPInput::soakData(int fd,int *readed)  {
    int res,n,avail,nread;
    // reset the read count
    nread=*readed;
    *readed=0;
    // determine how much room we have to work with
    avail=getAvailableForWriting();
    ///*DEBUG*/printf("soak: avail=%d\n",avail);
    // if we got nothing available, throw something
    if (!avail) {
        throw FPBufferFullException("In FPInput::soakData()");
    }
    if (puti>=geti) {
        // if puti is greater than upto, we should first read into puti
        // to the end of the buffer
        n=size-puti;
        ///*DEBUG*/printf("soak-flip: n=%d, nread=%d\n",n,nread);
        for (;;) {
            res=read(fd,queue+puti,FP_min(n,nread));
            if (res>=0||errno!=EINTR) break;
        }
        // if res==0 then EOF
        if (res==0) {
            throw FPEOFException("In FPInput::soakData()");
        }
        // if res<0 then we have an error, so we should exit
        if (res<0) {
            throw FPSystemException(errno,
                    "In FPInput::soakData()");
        }
        // set the read count
        *readed=res;
        nread-=res;
        if (res>0) post_empty=false;
        // if res<n then we didn't read all that we wanted, so we should exit
        if (res<n) {
            addSoak(*readed);
            puti+=res;
            return;
        }
        // set puti to zero, since we read to the end
        puti=0;
    }
    // if puti is smaller than geti, we should read the difference
    n=geti-puti;
    int to_read=FP_min(n,nread);
    if (to_read>0) {
        for (;;) {
            res=read(fd,queue+puti,to_read);
            if (res>=0||errno!=EINTR) break;
        }
        if (res==0) {
            addSoak(*readed);
            throw FPEOFException("In FPInput::soakData()");
        }
        // error - exit
        if (res<0) {
            addSoak(*readed);
            throw FPSystemException(errno,
                    "In FPInput::soakData()");
        }
        // increment
        puti+=res;
        *readed+=res;
        if (res>0) post_empty=false;
    }
    addSoak(*readed);
}

void FPInput::beginPacket()  {
    int i,j,n,avail;
    char *ptr=(char*)&n;
    ///*DEBUG*/printf("pre_empty=%d post_empty=%d\n",pre_empty,post_empty);
    if (!pre_empty||in_packet) {
        throw FPBadOperationException("In FPInput::beginPacket()");
    }
    if (post_empty) {
        throw FPNotEnoughDataException("In FPInput::beginPacket()");
    }
    avail=FPQueue_rollingDiff(puti,geti,size);
    ///*DEBUG*/printf("avail=%d\n",avail);
    if (avail<4) {
        // we don't even have a packet length header
        throw FPNotEnoughDataException("In FPInput::beginPacket()");
    }
    if (getEndianness()==ENDIANNESS_CONVERT) {
        for (i=geti,j=3;j>=0&&i<size;i++,j--) {
            ptr[j]=queue[i];
        }
        for (i=0;j>=0;i++,j--) {
            ptr[j]=queue[i];
        }
    } else {
        for (i=geti,j=0;j<4&&i<size;i++,j++) {
            ptr[j]=queue[i];
        }
        for (i=0;j<4;i++,j++) {
            ptr[j]=queue[i];
        }
    }
    ///*DEBUG*/printf("n=%d\n",n);
    if (n>size-4) {
        ///*DEBUG*/printf("ret=-2\n");
        throw FPPacketTooBigException("In FPInput::beginPacket()");
    }
    if (avail<4+n) {
        throw FPNotEnoughDataException("In FPInput::beginPacket()");
    }
    geti+=4;
    if (geti>=size) geti-=size;
    upto=geti+n;
    if (upto>=size) upto-=size;
    if (avail==4+n) post_empty=true;
    if (upto!=geti) pre_empty=false;
    in_packet=true;
}

void FPInput::beginPacketAndDecompress()  {
    throw FPNotImplementedException("In FPInput::beginPacketAndDecompress()");
}

bool FPInput::endPacket()  {
    bool ret=false;
    if (pre_empty) {
        if (in_packet) {
            // it's all good - we're ending the packet when we're already at its end
            ret=true;
        } else {
            throw FPNoPacketException("In FPInput::endPacket()");
        }
    }
    geti=upto;
    pre_empty=true;
    in_packet=false;
    return ret;
}

bool FPInput::hasPacket()  {
    int i,j,n,avail;
    char *ptr=(char*)&n;
    if (post_empty) return false;  // we explicitly know it's empty
    if (!pre_empty) return true;  // we already gots one
    avail=FPQueue_rollingDiff(puti,geti,size);
    if (avail<4) return false;    // we don't even have a packet length descriptor
    //printf("getEndianness()=%d\n",getEndianness());
    if (getEndianness()==ENDIANNESS_CONVERT) {
        //printf("converting!\n");
        for (i=geti,j=3;j>=0&&i<size;i++,j--) {
            ptr[j]=queue[i];
        }
        for (i=0;j>=0;i++,j--) {
            ptr[j]=queue[i];
        }
    } else {
        //printf("not converting!\n");
        for (i=geti,j=0;j<4&&i<size;i++,j++) {
            ptr[j]=queue[i];
        }
        for (i=0;j<4;i++,j++) {
            ptr[j]=queue[i];
        }
    }
    //printf("n=%d\n",n);
    if (n>size-4) {
        std::ostringstream buf;
        buf<<"In FPInput::hasPacket(): "<<n<<">"<<size<<"-4";
        throw FPPacketTooBigException(buf.str());
    }
    if (avail<4+n) return false;
    //printf("avail = %d, n = %d, size = %d\n",avail,n,size);
    return true;
}

const char *FPInput::requestInputSpan(int len) {
    if (pre_empty) {
        return NULL;
    }
    
    if (geti>=upto) {
        return NULL;
    }
    
    if (len>upto-geti) {
        return NULL;
    }
    
    const char *ret=queue+geti;
    
    geti+=len;
    if (geti==upto) {
        pre_empty=true;
    }
    
    return ret;
}

void FPInput::readBytes(char *data,int len)  {
    int n;
    ///*DEBUG*/printf("get=%d upto=%d\n",geti,upto);
    if (pre_empty) {
        if (in_packet) {
            if (len==0) {
                return;
            }
            throw FPNotEnoughDataException("In FPInput::readBytes()");
        } else {
            throw FPNoPacketException("In FPInput::readBytes()");
        }
    }
    if (geti>=upto) {
        n=size-geti;  // determine how much on mainside
        if (len>n) {  // there's more!
            if (len>n+upto) {
                // not enough data in the packet
                throw FPNotEnoughDataException("In FPInput::readBytes()");
            }
            memcpy(data,queue+geti,n);
            memcpy(data+n,queue,len-n);
            geti=len-n;
            if (geti==upto) pre_empty=true;
        } else {
            memcpy(data,queue+geti,len);
            geti+=len;
            if (geti==size) geti=0;
            if (geti==upto) pre_empty=true;
        }
    } else {
        if (len>upto-geti) {
            throw FPNotEnoughDataException("In FPInput::readBytes()");
        }
        memcpy(data,queue+geti,len);
        geti+=len;
        if (geti==upto) pre_empty=true;
    }
}

void FPInput::readString(char *data,int _len)  {
    int i=0,len=_len-1,old_geti;
    char c;
    if (pre_empty) {
        if (in_packet) {
            throw FPNotEnoughDataException("In FPInput::readString()");
        } else {
            throw FPNoPacketException("In FPInput::readString()");
        }
    }
    old_geti=geti;
    data[len]=0;
    for (;;) {
        c=queue[geti];
        if (i<=len) {
            data[i]=c;
            i++;
        } else {
            geti=old_geti;
            throw FPTargetFullException("In FPInput::readString()");
        }
        geti++;
        if (geti>=size) geti=0;
        if (!c) break;
        if (geti==upto) {
            geti=old_geti;
            throw FPBadFormatException("In FPInput::readString()");
        }
    }
    if (geti==upto) pre_empty=true;
}

void FPInput::readWordImpl(char *data,int len)  {
    int n,i,j;
    if (getEndianness()==ENDIANNESS_CONVERT) {
        if (pre_empty) {
            if (in_packet) {
                if (len==0) {
                    return;
                }
                throw FPNotEnoughDataException("In FPInput::readWordImpl()");
            } else {
                throw FPNoPacketException("In FPInput::readWordImpl()");
            }
        }
        if (geti>=upto) {
            n=size-geti;  // determine how much on mainside
            if (len>n) {  // there's more!
                if (len>n+upto) {
                    throw FPNotEnoughDataException(
                            "In FPInput::readWordImpl()");
                }
                for (i=geti,j=len-1;i<size;i++,j--) {
                    data[j]=queue[i];
                }
                for (i=0;j>=0;i++,j--) {
                    data[j]=queue[i];
                }
                geti=len-n;
                if (geti==upto) pre_empty=true;
            } else {
                for (i=geti,j=len-1;j>=0;i++,j--) {
                    data[j]=queue[i];
                }
                geti+=len;
                if (geti==size) geti=0;
                if (geti==upto) pre_empty=true;
            }
        } else {
            if (len>upto-geti) {
                throw FPNotEnoughDataException("In FPInput::readWordImpl()");
            }
            for (i=geti,j=len-1;j>=0;i++,j--) {
                data[j]=queue[i];
            }
            geti+=len;
            if (geti==upto) pre_empty=true;
        }
    } else {
        readBytes(data,len);
    }
}

void FPInput::getFirst(char **data,int *len) {
    int i=geti;
    *data=queue+i;
    if (upto>=i) *len=puti-i;
    else *len=size-i;
}

void FPInput::getSecond(char **data,int *len) {
    int i=geti;
    if (upto>=i) {
        *data=NULL;
        *len=0;
    } else {
        *data=queue;
        *len=upto;
    }
}

FPAutoOutput::FPAutoOutput(int _fd,int bytes)  :
    FPOutput(bytes),
    fd(_fd),
    blitz_count(0)
{
}

FPAutoOutput::FPAutoOutput(int bytes)  :
    FPOutput(bytes),
    fd(-1),
    blitz_count(0)
{
}

FPAutoOutput::~FPAutoOutput() {
    try {
        flushData();
    } catch (const FPException &e) {}
}

void FPAutoOutput::beginPacket()  {
    if (getAvailableForWritingUnsafe()<getRequiredForBeginPacket()) {
        flushData();
    }
    FPOutput::beginPacket();
}

int FPAutoOutput::putBytes(const char *data,int len)  {
    if (getAvailableForWritingUnsafe()<len) {
        flushData();
    }
    return FPOutput::putBytes(data,len);
}

void FPAutoOutput::writeBytes(const char *data,int len)  {
    if (getAvailableForWritingUnsafe()<len) {
        flushData();
    }
    FPOutput::writeBytes(data,len);
}

void FPAutoOutput::writePadding(int len)  {
    if (getAvailableForWritingUnsafe()<len) {
        flushData();
    }
    FPOutput::writePadding(len);
}

void FPAutoOutput::writeWordImpl(const char *data,int len)  {
    if (getAvailableForWritingUnsafe()<len) {
        flushData();
    }
    FPOutput::writeWordImpl(data,len);
}

void FPAutoOutput::writeString(const char *data)  {
    try {
        FPOutput::writeString(data);
    } catch (const FPBufferFullException &e) {
        flushData();
        FPOutput::writeString(data);
    } catch (const FPException &e) {
        throw;
    }
}

void FPAutoOutput::flushData()  {
    int total_len=0;
    while (hasPre()) {
        int len;
        blitzData(fd,&len);
        total_len+=len;
    }
    blitz_count+=total_len;
}

uword64 FPAutoOutput::getBlitzCount() {
    uword64 ret=blitz_count;
    blitz_count=0;
    return ret;
}

FPInterruptableInput::FPInterruptableInput(int bytes)  :
    FPInput(bytes)
{
    if (::pipe(my_pipe)<0) {
        throw FPSystemException(errno,
            "In FPInterruptableInput::FPInterruptableInput(): "
            "creating pipe");
    }
}

FPInterruptableInput::~FPInterruptableInput() {
    close(my_pipe[0]);
    close(my_pipe[1]);
}

void FPInterruptableInput::soakNewPacket(int fd,int *read)  {
    int nread,throwaway;
    fd_set reads;
    if (read==NULL) read=&throwaway;
    *read=0;
    while (!hasPacket()) {
        FD_ZERO(&reads);
        FD_SET(fd,&reads);
        FD_SET(my_pipe[0],&reads);
        int res=::select((fd>my_pipe[0]?fd:my_pipe[0])+1,
                         &reads,NULL,NULL,NULL);
        
        if (res<0) {
            if (errno==EINTR) {
                continue;
            }
            throw FPSystemException(errno,
                "In FPInterruptableInput::soakNewPacket()");
        }
        
        if (res==0) {
            throw FPInternalException(
                "::select() without timeout returned zero");
        }
        
        if (FD_ISSET(my_pipe[0],&reads)) {
            char c;
            int res;
            if ((res=::read(my_pipe[0],&c,1))<1) {
                if (res<0) {
                    throw FPSystemException(errno,
                        "In FPInterruptableInput::soakNewPacket(): "
                        "while reading from interrupt pipe");
                } else {
                    throw FPInternalException(
                        "In FPInterruptableInput::soakNewPacket(): "
                        "interrupt pipe closed");
                }
            }
            
            throw FPInterruptedException(
                "In FPInterruptableInput::soakNewPacket() by "
                "FPInterruptableInput::interruptSoakNewPacket()");
        }
        
        if (FD_ISSET(fd,&reads)) {
            nread=getAvailableForWriting();
            soakData(fd,&nread);
            *read+=nread;
        }
    }
}

void FPInterruptableInput::interruptSoakNewPacket()  {
    char c;
    if (::write(my_pipe[1],&c,1)<0) {
        throw FPSystemException(errno,
            "In FPInterruptableInput::interruptSoakNewPacket(): "
            "while writing to interrupt pipe");
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
