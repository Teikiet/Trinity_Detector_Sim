/*
 * VDatum.cpp
 * by Filip Pizlo, 2005
 */

#include "VDatum.h"
#include "VWordParsing.h"
#include "VSampleCompressor.h"
#include "VSampleDecompressor.h"

#include <typeinfo>
#include <iostream>

using namespace std;

VDatum::Version VDatum::parseVersion(const std::string &str) {
    if (str=="AUG_2004") {
	return AUG_2004;
    } else if (str=="AUG_2005") {
	return AUG_2005;
    } else {
	throw VDatumVersionParseException(str);
    }
}

std::string VDatum::versionToStr(VDatum::Version version) {
    switch (version) {
    case AUG_2004: return "AUG_2004";
    case AUG_2005: return "AUG_2005";
    default: V_FAIL("bad version");
    }
}

uint8_t *VDatum::readFromBufferImpl(uint8_t *cur,
				    uint8_t *end,
				    VDatum::Version version) {
    switch (version) {
    case AUG_2004:
	if (end-cur<20) {
	    throw VDatumSizeInvalidException();
	}
	break;
    case AUG_2005:
	if (end-cur<16) {
	    throw VDatumSizeInvalidException();
	}
	break;
    default:
	V_FAIL("bad version");
    }
    copy_ntohl_incsrc((uint8_t*)&eventNumber,cur);
    for (unsigned i=0;i<5;++i) {
	copy_ntohs_incsrc((uint8_t*)&(GPSTime[i]),cur);
    }
    copy_ntohc_incsrc(&GPSYear,cur);
    copy_ntohc_incsrc(&eventType,cur);
    switch (version) {
    case AUG_2004:
	copy_ntohl_incsrc((uint8_t*)&flags,cur);
	break;
    case AUG_2005:
	break;
    default:
	V_FAIL("bad version");
    }
    return cur;
}

uint8_t *VDatum::readFromBuffer(uint8_t *cur,
				uint8_t *end,
				VDatum::Version version) {
    this->version=version;
    uint8_t *result=readFromBufferImpl(cur,end,version);
    V_ASSERT(result<=end);
    wilderness_size=end-result;
    if (wilderness_size>0) {
	wilderness=new uint8_t[wilderness_size];
	memcpy(wilderness,result,wilderness_size);
    } else {
	wilderness=NULL;
    }
    return result;
}

uint8_t *VDatum::writeToBufferImpl(uint8_t *cur) {
    copy_htonl_incdst(cur,(uint8_t*)&eventNumber);
    for (unsigned i=0;i<5;++i) {
	copy_htons_incdst(cur,(uint8_t*)&(GPSTime[i]));
    }
    copy_htonc_incdst(cur,&GPSYear);
    copy_htonc_incdst(cur,&eventType);
    switch (getVersion()) {
    case AUG_2004:
	copy_htonl_incdst(cur,(uint8_t*)&flags);
	break;
    case AUG_2005:
	break;
    default:
	V_FAIL("bad version");
    }
    return cur;
}

uint8_t *VDatum::writeToBuffer(uint8_t *cur) {
    uint8_t *begin=cur;
    cur=writeToBufferImpl(cur);
    memcpy(cur,wilderness,wilderness_size);
    cur+=wilderness_size;
    uword32 size;
    switch (getVersion()) {
    case AUG_2004:
	size=getSize()-8;
	break;
    case AUG_2005:
	size=getSize()-12;
	break;
    default:
	V_FAIL("bad version");
    }
    V_ASSERT((unsigned)(cur-begin)==size);
    return cur;
}

VDatum::VDatum():
    version(currentVersion()),
    wilderness_size(0),
    wilderness(NULL),
    flags(0)
{}

VDatum::~VDatum() {
    if (wilderness!=NULL) {
	delete wilderness;
    }
}

uword32 VDatum::getSize() const {
    return 28+wilderness_size;
}

void VDatum::writeTo(FPOutputBase *output) {
    uword32 size;
    if (version==AUG_2004) {
	size=getSize()-8;
    } else {
	size=getSize()-12;
    }
    output->writeWord((uint8_t)'V');
    output->writeWord((uint8_t)'E');
    output->writeWord((uint8_t)'V');
    output->writeWord((uint8_t)'N');
    output->writeWord(nodeNumber);
    output->writeWord(triggerMask);
    switch (version) {
    case AUG_2004:
	output->writeWord((uword16)size);
	break;
    case AUG_2005:
	output->writeWord((uword16)flags);
	output->writeWord((uword32)size);
	break;
    default:
	V_FAIL("bad version");
    }
    uint8_t *buf=(uint8_t*)output->requestOutputSpan(size);
    if (buf==NULL) {
	buf=new uint8_t[size];
	try {
	    writeToBuffer(buf);
	    output->writeBytes((const char*)buf,size);
	    delete[] buf;
	} catch (...) {
	    delete[] buf;
	    throw;
	}
    } else {
	writeToBuffer(buf);
    }
}

void VDatum::readFrom(FPInputBase *input,
		      VDatum::Version version) {
    if (input->readUByte()!='V' ||
	input->readUByte()!='E' ||
	input->readUByte()!='V' ||
	input->readUByte()!='N') {
	throw VDatumInvalidException("Bad magic number");
    }
    nodeNumber=input->readUByte();
    triggerMask=input->readUByte();
    uword32 size;
    switch (version) {
    case AUG_2004:
	size=input->readUWord16();
	break;
    case AUG_2005:
	flags=input->readUWord16();
	size=input->readUWord32();
	break;
    default:
	V_FAIL("bad version");
    }
    uint8_t *buf=(uint8_t*)input->requestInputSpan(size);
    if (buf==NULL) {
	buf=new uint8_t[size];
	try {
	    input->readBytes((char*)buf,size);
	    readFromBuffer(buf,buf+size,version);
	    delete[] buf;
	} catch (...) {
	    delete[] buf;
	    throw;
	}
    } else {
	readFromBuffer(buf,buf+size,version);
    }
}

VDatum *VDatum::copy() const {
    V_FAIL("cannot copy a VDatum");
}

bool VDatum::isEqualTo(const VDatum *other) const {
    if (typeid(*this)!=typeid(*other)) {
	return false;
    }
    return isEqualToImpl(other);
}

bool VDatum::isEqualToImpl(const VDatum *other) const {
    return version==other->version
	&& wilderness_size==other->wilderness_size
	&& !memcmp(wilderness,other->wilderness,wilderness_size)
	&& nodeNumber==other->nodeNumber
	&& triggerMask==other->triggerMask
	&& eventNumber==other->eventNumber
	&& !memcmp(GPSTime,other->GPSTime,10)
	&& GPSYear==other->GPSYear
	&& eventType==other->eventType
	&& flags==other->flags;
}

void VDatum::setFlags(uword32 flags) {
    this->flags=flags;
}

void VEvent::buildCS() const {
    if (!csDirty) {
	return;
    }
    if (cs!=NULL) {
	delete[] cs;
	cs=NULL;
    }
    cpIndices.clear();
    VSampleCompressor sc(numSamples,numChannels);
    for (unsigned i=0;i<numChannels;++i) {
	uint8_t *trace=samples+i*numSamples;
	unsigned charge=0;
	for (unsigned j=numSamples;j-->0;) {
	    charge+=trace[j];
	}
	bool xtra_bit=false;
	if (pedsAndHiLo[i]!=0 || charges[i]!=charge) {
	    xtra_bit=true;
	    cpIndices.push_back(i);
	}
	sc.add(trace,xtra_bit);
    }
    cs=sc.begin();
    csSize=sc.size();
    sc.releaseBuffer();
    csDirty=false;
}

void VEvent::buildHitVec() const {
    if (hvDirty) {
	hitVec.clear();
	for (unsigned i=0;i<getMaxNumChannels();++i) {
	    if (getHitBit(i)) {
		hitVec.push_back(i);
	    }
	}
	hvDirty=false;
    }
}

uint8_t *VEvent::readFromBufferImpl(uint8_t *cur,
				    uint8_t *end,
				    VDatum::Version version) {
    csDirty=true;
    cur=VDatum::readFromBufferImpl(cur,end,version);
    switch (version) {
    case AUG_2004:
	if (end-cur<8) {
	    throw VDatumSizeInvalidException();
	}
	break;
    case AUG_2005:
	if (end-cur<12) {
	    throw VDatumSizeInvalidException();
	}
	break;
    default:
	V_FAIL("bad version");
    }
    uword16 numSamples,numChannels,maxNumChannels,numClockTrigBoards;
    if (version==AUG_2005) {
	copy_ntohl_incsrc((uint8_t*)&dotModule,cur);
    }
    copy_ntohs_incsrc((uint8_t*)&numSamples,cur);
    if (version==AUG_2005) {
	compressed=(getFlags()&1);
    } else {
	if ((numSamples>>15)&1) {
	    compressed=true;
	    numSamples&=~(1<<15);
	} else {
	    compressed=false;
	}
    }
    copy_ntohs_incsrc((uint8_t*)&numChannels,cur);
    copy_ntohs_incsrc((uint8_t*)&maxNumChannels,cur);
    copy_ntohs_incsrc((uint8_t*)&numClockTrigBoards,cur);
    resizeChannelData(numSamples,numChannels);
    resizeChannelBits(maxNumChannels);
    resizeClockTrigData(numClockTrigBoards);
    if (willUseCompression()) {
	if (end-cur<(int)getEventPreSampleVarBodySize()) {
	    throw VDatumSizeInvalidException("At pre-bits compress mode check");
	}
    } else {
	if (end-cur<(int)getEventVarBodySize()) {
	    ostringstream buf;
	    buf<<"At pre-var-body check: end-cur = "
	       <<(end-cur)<<"; var body size = "<<getEventVarBodySize();
	    throw VDatumSizeInvalidException(buf.str());
	}
    }
    for (unsigned i=0;i<numBitPatternElements();++i) {
	copy_ntohl_incsrc((uint8_t*)(hitPattern+i),cur);
    }
    for (unsigned i=0;i<numBitPatternElements();++i) {
	copy_ntohl_incsrc((uint8_t*)(triggerPattern+i),cur);
    }
    if (willUseCompression()) {
	VSampleDecompressor sd(numSamples,cur,end);
	vector< uword16 > indices;
	for (unsigned i=0;i<numChannels;++i) {
	    bool xtra_bit;
	    uint8_t *trace=samples+i*numSamples;
	    sd.get(trace,xtra_bit);
	    if (xtra_bit) {
		indices.push_back(i);
	    } else {
		charges[i]=0;
		for (unsigned j=numSamples;j-->0;) {
		    charges[i]+=trace[j];
		}
		pedsAndHiLo[i]=0;
	    }
	}
	cur+=((sd.size()+3)&~3);
	if (end-cur<(int)(4*indices.size()+
			  getEventPostSampleVarBodySize())) {
	    throw VDatumSizeInvalidException();
	}
	for (unsigned i=0;i<indices.size();++i) {
	    copy_ntohs_incsrc((uint8_t*)(pedsAndHiLo+indices[i]),cur);
	    copy_ntohs_incsrc((uint8_t*)(charges+indices[i]),cur);
	}
    } else {
	uint8_t *curSample=samples;
	for (unsigned i=0;i<numChannels;++i) {
	    memcpy(curSample,cur,numSamples);
	    cur+=numSamples;
	    curSample+=numSamples;
	    copy_ntohs_incsrc((uint8_t*)(pedsAndHiLo+i),cur);
	    copy_ntohs_incsrc((uint8_t*)(charges+i),cur);
	}
    }
    uword32 *curCTB=clockTrigData;
    for (unsigned i=0;i<numClockTrigBoards;++i) {
	for (unsigned j=7;j-->0;) {
	    copy_ntohl_incsrc((uint8_t*)curCTB,cur);
	    curCTB++;
	}
    }
    return cur;
}

uint8_t *VEvent::writeToBufferImpl(uint8_t *cur) {
    cur=VDatum::writeToBufferImpl(cur);
    uword16 numSamples=this->numSamples;
    if (compressed && getVersion()==AUG_2004) {
	numSamples|=(1<<15);
    }
    if (getVersion()>=AUG_2005) {
	copy_htonl_incdst(cur,(uint8_t*)&dotModule);
    }
    copy_htons_incdst(cur,(uint8_t*)&numSamples);
    copy_htons_incdst(cur,(uint8_t*)&numChannels);
    copy_htons_incdst(cur,(uint8_t*)&maxNumChannels);
    copy_htons_incdst(cur,(uint8_t*)&numClockTrigBoards);
    for (unsigned i=0;i<numBitPatternElements();++i) {
	copy_htonl_incdst(cur,(uint8_t*)(hitPattern+i));
    }
    for (unsigned i=0;i<numBitPatternElements();++i) {
	copy_htonl_incdst(cur,(uint8_t*)(triggerPattern+i));
    }
    if (willUseCompression()) {
	buildCS();
	memcpy(cur,cs,csSize);
	cur+=((csSize+3)&~3);
	for (unsigned i=0;i<cpIndices.size();++i) {
	    copy_htons_incdst(cur,(uint8_t*)(pedsAndHiLo+cpIndices[i]));
	    copy_htons_incdst(cur,(uint8_t*)(charges+cpIndices[i]));
	}
    } else {
	uint8_t *curSample=samples;
	for (unsigned i=0;i<numChannels;++i) {
	    memcpy(cur,curSample,numSamples);
	    cur+=numSamples;
	    curSample+=numSamples;
			
	    copy_htons_incdst(cur,(uint8_t*)(pedsAndHiLo+i));
	    copy_htons_incdst(cur,(uint8_t*)(charges+i));
	}
    }
    uword32 *curCTB=clockTrigData;
    for (unsigned i=0;i<numClockTrigBoards;++i) {
	for (unsigned j=7;j-->0;) {
	    copy_htonl_incdst(cur,(uint8_t*)curCTB);
	    curCTB++;
	}
    }
    return cur;
}

#define VEvent_init				\
    compressed(false),				\
    numSamples(0),				\
    numChannels(0),				\
    maxNumChannels(0),			        \
    numClockTrigBoards(0),			\
    hitPattern(NULL),			        \
    triggerPattern(NULL),			\
    pedsAndHiLo(NULL),			        \
    charges(NULL),				\
    samples(NULL),				\
    cs(NULL),                                   \
    csSize(0),                                  \
    csDirty(true),                              \
    clockTrigData(NULL),                        \
    selectedChan(0),                            \
    hvDirty(true)

VEvent::VEvent():
    VDatum(),
    VEvent_init
{}

VEvent::VEvent(uword16 numSamples,
	       uword16 numChannels,
	       uword16 maxNumChannels,
	       uword16 numClockTrigBoards):
    VDatum(),
    VEvent_init
{
    resizeChannelData(numSamples,numChannels);
    resizeChannelBits(maxNumChannels);
    resizeClockTrigData(numClockTrigBoards);
}

VEvent::~VEvent() {
    if (hitPattern!=NULL) {
	free(hitPattern);
    }
    if (triggerPattern!=NULL) {
	free(triggerPattern);
    }
    if (pedsAndHiLo!=NULL) {
	free(pedsAndHiLo);
    }
    if (charges!=NULL) {
	free(charges);
    }
    if (samples!=NULL) {
	free(samples);
    }
    if (clockTrigData!=NULL) {
	free(clockTrigData);
    }
    if (cs!=NULL) {
	delete[] cs;
    }
}

VEvent *VEvent::copyEvent() const {
    VEvent *result=copyImpl< VEvent >();
    result->resizeChannelData(numSamples,numChannels);
    result->resizeChannelBits(maxNumChannels);
    result->resizeClockTrigData(numClockTrigBoards);
    result->dotModule=dotModule;
    result->compressed=compressed;
    memcpy(result->hitPattern,hitPattern,numBitPatternElements()*4);
    memcpy(result->triggerPattern,triggerPattern,numBitPatternElements()*4);
    memcpy(result->pedsAndHiLo,pedsAndHiLo,numChannels*2);
    memcpy(result->charges,charges,numChannels*2);
    memcpy(result->samples,samples,numChannels*numSamples);
    memcpy(result->clockTrigData,clockTrigData,7*4*numClockTrigBoards);
    return result;
}

VDatum *VEvent::copy() const {
    return copyEvent();
}

uword32 VEvent::getSize() const {
    switch (getVersion()) {
    case AUG_2004:
	return VDatum::getSize()+8+getEventVarBodySize();
    case AUG_2005:
	return VDatum::getSize()+12+getEventVarBodySize();
    default:
	V_FAIL("bad version");
    }
}

bool VEvent::hasFlags() {
    return getVersion()>=AUG_2005;
}

bool VEvent::isFlags16Bit() {
    return getVersion()>=AUG_2005;
}

bool VEvent::isFlags32Bit() {
    return false;
}

void VEvent::setFlags(uword32 flags) {
    if (getVersion()>=AUG_2005) {
	compressed=(flags&1);
    }
    VDatum::setFlags(flags);
}

void VEvent::resizeChannelData(uword16 numSamples,
			       uword16 numChannels) {
    csDirty=true;
    
    // first manage samples array
    if (this->numSamples!=numSamples &&
	this->numChannels!=numChannels &&
	((this->numSamples!=0 &&
	  this->numChannels!=0) ||
	 (numSamples!=0 &&
	  numChannels!=0))) {
	if (this->numSamples==0 ||
	    this->numChannels==0) {
	    samples=(uint8_t*)malloc(numSamples*numChannels);
	} else if (numSamples==0 ||
		   numChannels==0) {
	    free(samples);
	    samples=NULL;
	} else if (numSamples==this->numSamples) {
	    samples=(uint8_t*)realloc(samples,numSamples*numChannels);
	} else {
	    uint8_t *newSamples=(uint8_t*)malloc(numSamples*numChannels);
	    for (unsigned i=0;i<this->numChannels;++i) {
		for (unsigned j=0;j<this->numSamples;++j) {
		    newSamples[i*numChannels+j]=
			samples[i*this->numChannels+j];
		}
	    }
	    free(samples);
	    samples=newSamples;
	}
    }
	
    // now do the others
    if (this->numChannels!=numChannels) {
	if (this->numChannels==0) {
	    pedsAndHiLo=(uword16*)malloc(2*numChannels);
	    charges=(uword16*)malloc(2*numChannels);
	} else if (numChannels==0) {
	    free(pedsAndHiLo);
	    free(charges);
	    pedsAndHiLo=NULL;
	    charges=NULL;
	} else {
	    pedsAndHiLo=(uword16*)realloc(pedsAndHiLo,2*numChannels);
	    charges=(uword16*)realloc(charges,2*numChannels);
	}
    }
	
    this->numSamples=numSamples;
    this->numChannels=numChannels;
}

void VEvent::resizeChannelBits(uword16 maxNumChannels) {
    if (this->maxNumChannels==maxNumChannels) {
	return;
    }
    hvDirty=true;
    if (this->maxNumChannels==0) {
	this->maxNumChannels=maxNumChannels;
	hitPattern=(uword32*)malloc(numBitPatternElements()*4);
	triggerPattern=(uword32*)malloc(numBitPatternElements()*4);
    } else if (maxNumChannels==0) {
	free(hitPattern);
	free(triggerPattern);
	hitPattern=NULL;
	triggerPattern=NULL;
    } else {
	this->maxNumChannels=maxNumChannels;
	hitPattern=(uword32*)realloc(hitPattern,numBitPatternElements()*4);
	triggerPattern=(uword32*)realloc(triggerPattern,numBitPatternElements()*4);
    }
}

void VEvent::resizeClockTrigData(uword16 numClockTrigBoards) {
    if (this->numClockTrigBoards==numClockTrigBoards) {
	return;
    }
    if (this->numClockTrigBoards==0) {
	clockTrigData=(uword32*)malloc(7*4*numClockTrigBoards);
    } else if (numClockTrigBoards==0) {
	free(clockTrigData);
	clockTrigData=NULL;	
    } else {
	clockTrigData=(uword32*)realloc(clockTrigData,7*4*numClockTrigBoards);
    }
    this->numClockTrigBoards=numClockTrigBoards;
}

bool VEvent::isEqualToImpl(const VDatum *_other) const {
    if (!VDatum::isEqualToImpl(_other)) {
	return false;
    }
    VEvent *other=(VEvent*)_other;
    if (getVersion()>=AUG_2005) {
	if (dotModule!=other->dotModule) {
	    return false;
	}
    }
    return numSamples==other->numSamples
	&& numChannels==other->numChannels
	&& maxNumChannels==other->maxNumChannels
	&& numClockTrigBoards==other->numClockTrigBoards
	&& !memcmp(hitPattern,other->hitPattern,numBitPatternElements()*4)
	&& !memcmp(triggerPattern,other->triggerPattern,numBitPatternElements()*4)
	&& !memcmp(pedsAndHiLo,other->pedsAndHiLo,2*numChannels)
	&& !memcmp(charges,other->charges,2*numChannels)
	&& !memcmp(samples,other->samples,numChannels*numSamples)
	&& !memcmp(clockTrigData,other->clockTrigData,7*4*numClockTrigBoards);
}

uint32_t VEvent::getHitID(uint32_t i) const {
    buildHitVec();
    V_ASSERT(i<hitVec.size());
    return hitVec[i];
}

pair< bool, uint32_t > VEvent::getChannelHitIndex(uint32_t chan) const {
    V_ASSERT(chan<getMaxNumChannels());
    if (!getHitBit(chan)) {
	return pair< bool, uint32_t >(false,0);
    }
    unsigned j=0;
    for (unsigned i=0;
	 i<chan;
	 ++i) {
	if (getHitBit(i)) {
	    j++;
	}
    }
    return pair< bool, uint32_t >(true,j);
}

vector< bool > VEvent::getFullHitVec() const {
    vector< bool > result;
    for (unsigned i=0;i<getMaxNumChannels();++i) {
	result.push_back(getHitBit(i));
    }
    return result;
}

vector< bool > VEvent::getFullTrigVec() const {
    vector< bool > result;
    for (unsigned i=0;i<getMaxNumChannels();++i) {
	result.push_back(getTriggerBit(i));
    }
    return result;
}

vector< uint8_t > VEvent::getSamplesVec() const {
    vector< uint8_t > result;
    const uint8_t *cur=getSamplePtr(selectedChan,0);
    const uint8_t *end=getSamplePtr(selectedChan,getNumSamples());
    while (cur!=end) {
	result.push_back(*cur++);
    }
    return result;
}

void VEvent::selectHitChan(uint32_t chan) {
    verifyChannel(chan);
    selectedChan=chan;
}

uint8_t *VArrayTrigger::readFromBufferImpl(uint8_t *cur,
					   uint8_t *end,
					   Version version) {
    cur=VDatum::readFromBufferImpl(cur,end,version);
    if (end-cur<8) {
	throw VDatumSizeInvalidException();
    }
    switch (version) {
    case AUG_2004: {
	uint8_t numTelescopes;
	copy_ntohs_incsrc((uint8_t*)&ATflags,cur);
	copy_ntohc_incsrc(&configMask,cur);
	copy_ntohc_incsrc(&numTelescopes,cur);
	copy_ntohl_incsrc((uint8_t*)&runNumber,cur);
	resizeSubarrayTelescopes(numTelescopes);
	resizeTriggerTelescopes(numTelescopes);
	if (end-cur<(int)getATVarBodySize()) {
	    throw VDatumSizeInvalidException();
	}
	for (unsigned i=0;i<numTelescopes;++i) {
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].telId),cur);
	    trigTels[i].telId=subarrayTels[i].telId;
	    copy_ntohf_incsrc((uint8_t*)&(subarrayTels[i].altitude),cur);
	    copy_ntohf_incsrc((uint8_t*)&(subarrayTels[i].azimuth),cur);
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].tdcTime),cur);
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].delay),cur);
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].l2ScalarRate),cur);
	    for (unsigned j=0;j<3;++j) {
		copy_ntohl_incsrc((uint8_t*)(subarrayTels[i].l2Pattern+j),cur);
	    }
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].tenMhzClock),cur);
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].vetoedClock),cur);
	}
	break;
    }
    case AUG_2005: {
	uint8_t ATflags;
	uint8_t numSubarrayTelescopes;
	uint8_t numTriggerTelescopes;
	copy_ntohc_incsrc(&ATflags,cur);
	this->ATflags=ATflags;
	copy_ntohc_incsrc(&configMask,cur);
	copy_ntohc_incsrc(&numSubarrayTelescopes,cur);
	copy_ntohc_incsrc(&numTriggerTelescopes,cur);
	copy_ntohl_incsrc((uint8_t*)&runNumber,cur);
	for (unsigned i=0;i<4;++i) {
	    copy_ntohl_incsrc((uint8_t*)(tenMHzClock+i),cur);
	}
	for (unsigned i=0;i<3;++i) {
	    copy_ntohl_incsrc((uint8_t*)(optCalCount+i),cur);
	}
	for (unsigned i=0;i<3;++i) {
	    copy_ntohl_incsrc((uint8_t*)(pedCount+i),cur);
	}
	resizeSubarrayTelescopes(numSubarrayTelescopes);
	resizeTriggerTelescopes(numTriggerTelescopes);
	if (end-cur<(int)getATVarBodySize()) {
	    throw VDatumSizeInvalidException();
	}
	for (unsigned i=0;i<numSubarrayTelescopes;++i) {
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].telId),cur);
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].tdcTime),cur);
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].evType),cur);
	    copy_ntohf_incsrc((uint8_t*)&(subarrayTels[i].azimuth),cur);
	    copy_ntohf_incsrc((uint8_t*)&(subarrayTels[i].altitude),cur);
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].showerDelay),cur);
	    copy_ntohl_incsrc((uint8_t*)&(subarrayTels[i].compDelay),cur);
	    for (unsigned j=0;j<3;++j) {
		copy_ntohl_incsrc((uint8_t*)(subarrayTels[i].l2Counts+j),cur);
	    }
	    for (unsigned j=0;j<3;++j) {
		copy_ntohl_incsrc((uint8_t*)(subarrayTels[i].calCounts+j),cur);
	    }
	}
	for (unsigned i=0;i<numTriggerTelescopes;++i) {
	    copy_ntohl_incsrc((uint8_t*)&(trigTels[i].telId),cur);
	}
	break;
    }
    default:
	V_FAIL("bad version");
    }
    return cur;
}

uint8_t *VArrayTrigger::writeToBufferImpl(uint8_t *cur) {
    cur=VDatum::writeToBufferImpl(cur);
    switch (getVersion()) {
    case AUG_2004: {
	copy_htons_incdst(cur,(uint8_t*)&ATflags);
	copy_htonc_incdst(cur,&configMask);
	copy_htonc_incdst(cur,&numSubarrayTelescopes);
	copy_htonl_incdst(cur,(uint8_t*)&runNumber);
	for (unsigned i=0;i<numSubarrayTelescopes;++i) {
	    copy_htonl_incdst(cur,(uint8_t*)&(subarrayTels[i].telId));
	    copy_htonf_incdst(cur,(uint8_t*)&(subarrayTels[i].altitude));
	    copy_htonf_incdst(cur,(uint8_t*)&(subarrayTels[i].azimuth));
	    copy_htonl_incdst(cur,(uint8_t*)&(subarrayTels[i].tdcTime));
	    copy_htonl_incdst(cur,(uint8_t*)&(subarrayTels[i].delay));
	    copy_htonl_incdst(cur,(uint8_t*)&(subarrayTels[i].l2ScalarRate));
	    for (unsigned j=0;j<3;++j) {
		copy_htonl_incdst(cur,(uint8_t*)(subarrayTels[i].l2Pattern+j));
	    }
	    copy_htonl_incdst(cur,(uint8_t*)&(subarrayTels[i].tenMhzClock));
	    copy_htonl_incdst(cur,(uint8_t*)&(subarrayTels[i].vetoedClock));
	}
	break;
    }
    case AUG_2005: {
	uint8_t ATflags=(uint8_t)this->ATflags;
	copy_htonc_incdst(cur,&ATflags);
	copy_htonc_incdst(cur,&configMask);
	copy_htonc_incdst(cur,&numSubarrayTelescopes);
	copy_htonc_incdst(cur,&numTriggerTelescopes);
	copy_htonl_incdst(cur,(uint8_t*)&runNumber);
	for (unsigned i=0;i<4;++i) {
	    copy_htonl_incdst(cur,(uint8_t*)(tenMHzClock+i));
	}
	for (unsigned i=0;i<3;++i) {
	    copy_htonl_incdst(cur,(uint8_t*)(optCalCount+i));
	}
	for (unsigned i=0;i<3;++i) {
	    copy_htonl_incdst(cur,(uint8_t*)(pedCount+i));
	}
	for (unsigned i=0;i<numSubarrayTelescopes;++i) {
	    copy_ntohl_incdst(cur,(uint8_t*)&(subarrayTels[i].telId));
	    copy_ntohl_incdst(cur,(uint8_t*)&(subarrayTels[i].tdcTime));
	    copy_ntohl_incdst(cur,(uint8_t*)&(subarrayTels[i].evType));
	    copy_ntohf_incdst(cur,(uint8_t*)&(subarrayTels[i].azimuth));
	    copy_ntohf_incdst(cur,(uint8_t*)&(subarrayTels[i].altitude));
	    copy_ntohl_incdst(cur,(uint8_t*)&(subarrayTels[i].showerDelay));
	    copy_ntohl_incdst(cur,(uint8_t*)&(subarrayTels[i].compDelay));
	    for (unsigned j=0;j<3;++j) {
		copy_ntohl_incdst(cur,(uint8_t*)(subarrayTels[i].l2Counts+j));
	    }
	    for (unsigned j=0;j<3;++j) {
		copy_ntohl_incdst(cur,(uint8_t*)(subarrayTels[i].calCounts+j));
	    }
	}
	for (unsigned i=0;i<numTriggerTelescopes;++i) {
	    copy_ntohl_incdst(cur,(uint8_t*)&(trigTels[i].telId));
	}
	break;
    }
    default:
	V_FAIL("bad version");
    }
    return cur;
}

VArrayTrigger::VArrayTrigger():
    VDatum(),
    numSubarrayTelescopes(0),
    numTriggerTelescopes(0),
    subarrayTels(NULL),
    trigTels(NULL)
{
    setNodeNumber(V_ARRAY_TRIGGER_NODE);
}

VArrayTrigger::VArrayTrigger(uint8_t numSubarrayTelescopes,
			     uint8_t numTriggerTelescopes):
    VDatum(),
    numSubarrayTelescopes(0),
    numTriggerTelescopes(0),
    subarrayTels(NULL),
    trigTels(NULL)
{
    setNodeNumber(V_ARRAY_TRIGGER_NODE);
    resizeSubarrayTelescopes(numSubarrayTelescopes);
    resizeTriggerTelescopes(numTriggerTelescopes);
}

VArrayTrigger::~VArrayTrigger() {
    if (subarrayTels!=NULL) {
	free(subarrayTels);
    }
    if (trigTels!=NULL) {
	free(trigTels);
    }
}

VArrayTrigger *VArrayTrigger::copyAT() const {
    VArrayTrigger *result=copyImpl< VArrayTrigger >();
    result->ATflags=ATflags;
    result->configMask=configMask;
    result->runNumber=runNumber;
    result->resizeSubarrayTelescopes(numSubarrayTelescopes);
    result->resizeTriggerTelescopes(numTriggerTelescopes);
    memcpy(result->tenMHzClock,tenMHzClock,sizeof(tenMHzClock));
    memcpy(result->optCalCount,tenMHzClock,sizeof(optCalCount));
    memcpy(result->pedCount,tenMHzClock,sizeof(pedCount));
    memcpy(result->subarrayTels,subarrayTels,
	   sizeof(SubarrayTel)*numSubarrayTelescopes);
    memcpy(result->trigTels,trigTels,
	   sizeof(TrigTel)*numTriggerTelescopes);
    return result;
}

VDatum *VArrayTrigger::copy() const {
    return copyAT();
}

uword32 VArrayTrigger::getSize() const {
    uword32 result=VDatum::getSize()+8+getATVarBodySize();
    if (getVersion()>=AUG_2005) {
	result+=4*4+4*3+4*3;
    }
    return result;
}

bool VArrayTrigger::hasFlags() {
    return true;
}

bool VArrayTrigger::isFlags16Bit() {
    return getVersion()>=AUG_2005;
}

bool VArrayTrigger::isFlags32Bit() {
    return getVersion()==AUG_2004;
}

void VArrayTrigger::resizeSubarrayTelescopes(uint8_t numSubarrayTelescopes) {
    if (this->numSubarrayTelescopes==numSubarrayTelescopes) {
	return;
    }
    if (this->numSubarrayTelescopes==0) {
	subarrayTels=(SubarrayTel*)malloc(sizeof(SubarrayTel)*numSubarrayTelescopes);
    } else if (numSubarrayTelescopes==0) {
	free(subarrayTels);
	subarrayTels=NULL;
    } else {
	subarrayTels=(SubarrayTel*)
	    realloc(subarrayTels,
		    sizeof(SubarrayTel)*numSubarrayTelescopes);
    }
    this->numSubarrayTelescopes=numSubarrayTelescopes;
}

void VArrayTrigger::resizeTriggerTelescopes(uint8_t numTriggerTelescopes) {
    if (this->numTriggerTelescopes==numTriggerTelescopes) {
	return;
    }
    if (this->numTriggerTelescopes==0) {
	trigTels=(TrigTel*)malloc(sizeof(TrigTel)*numTriggerTelescopes);
    } else if (numTriggerTelescopes==0) {
	free(trigTels);
	trigTels=NULL;
    } else {
	trigTels=(TrigTel*)
	    realloc(trigTels,
		    sizeof(TrigTel)*numTriggerTelescopes);
    }
    this->numTriggerTelescopes=numTriggerTelescopes;
}

bool VArrayTrigger::isEqualToImpl(const VDatum *_other) const {
    if (!VDatum::isEqualToImpl(_other)) {
	return false;
    }
    VArrayTrigger *other=(VArrayTrigger*)_other;
    if (ATflags!=other->ATflags ||
	configMask!=other->configMask ||
	numSubarrayTelescopes!=other->numSubarrayTelescopes ||
	numTriggerTelescopes!=other->numTriggerTelescopes ||
	runNumber!=other->runNumber) {
	return false;
    }
    if (getVersion()==AUG_2005) {
	if (memcmp(tenMHzClock,other->tenMHzClock,sizeof(tenMHzClock)) ||
	    memcmp(optCalCount,other->optCalCount,sizeof(optCalCount)) ||
	    memcmp(pedCount,other->pedCount,sizeof(pedCount))) {
	    return false;
	}
    }
    for (unsigned i=numSubarrayTelescopes;i-->0;) {
	if (subarrayTels[i].telId!=other->subarrayTels[i].telId ||
	    subarrayTels[i].tdcTime!=other->subarrayTels[i].tdcTime ||
	    subarrayTels[i].azimuth!=other->subarrayTels[i].azimuth ||
	    subarrayTels[i].altitude!=other->subarrayTels[i].altitude) {
	    return false;
	}
	if (getVersion()==AUG_2004) {
	    if (subarrayTels[i].l2ScalarRate!=other->subarrayTels[i].delay ||
		subarrayTels[i].l2ScalarRate!=other->subarrayTels[i].l2ScalarRate ||
		memcmp(&(subarrayTels[i].l2Pattern),
		       &(other->subarrayTels[i].l2Pattern),
		       sizeof(uword32[3])) ||
		subarrayTels[i].tenMhzClock!=other->subarrayTels[i].tenMhzClock ||
		subarrayTels[i].vetoedClock!=other->subarrayTels[i].vetoedClock) {
		return false;
	    }
	}
	if (getVersion()==AUG_2005) {
	    if (subarrayTels[i].evType!=other->subarrayTels[i].evType ||
		subarrayTels[i].showerDelay!=other->subarrayTels[i].showerDelay ||
		subarrayTels[i].compDelay!=other->subarrayTels[i].compDelay ||
		memcmp(&(subarrayTels[i].l2Counts),
		       &(other->subarrayTels[i].l2Counts),
		       sizeof(uword32[3])) ||
		memcmp(&(subarrayTels[i].calCounts),
		       &(other->subarrayTels[i].calCounts),
		       sizeof(uword32[3]))) {
		return false;
	    }
	}
    }
    for (unsigned i=numTriggerTelescopes;i-->0;) {
	if (trigTels[i].telId!=other->trigTels[i].telId) {
	    return false;
	}
    }
    return true;
}

VDatum *VDatumParser::buildFrom(FPInputBase *input,
				VDatum::Version version) {
    if (input->readUByte()!='V' ||
	input->readUByte()!='E' ||
	input->readUByte()!='V' ||
	input->readUByte()!='N') {
	throw VDatumInvalidException("Bad magic number");
    }
    uint8_t nodeNumber=input->readUByte();
    uint8_t triggerMask=input->readUByte();
    uword32 size;
    uword16 flags=0;
    switch (version) {
    case VDatum::AUG_2004:
	size=input->readUWord16();
	break;
    case VDatum::AUG_2005:
	flags=input->readUWord16();
	size=input->readUWord32();
	break;
    default:
	V_FAIL("bad version");
    }
    VDatum *result;
    if (nodeNumber==V_ARRAY_TRIGGER_NODE) {
	result=new VArrayTrigger();
    } else {
	result=new VEvent();
    }
    result->setNodeNumber(nodeNumber);
    result->setTriggerMask(triggerMask);
    result->setFlags(flags);
    try {
	uint8_t *buf=(uint8_t*)input->requestInputSpan(size);
	if (buf==NULL) {
	    buf=new uint8_t[size];
	    try {
		input->readBytes((char*)buf,size);
		result->readFromBuffer(buf,buf+size,version);
		delete[] buf;
	    } catch (...) {
		delete[] buf;
		throw;
	    }
	} else {
	    result->readFromBuffer(buf,buf+size,version);
	}
    } catch (...) {
	delete result;
	throw;
    }
    return result;
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
