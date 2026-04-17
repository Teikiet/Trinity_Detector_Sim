# CMakeLists file for VBF
# by Juniper Foote, 2023
# Adapted from Makefile.am
# by Filip Pizlo, 2004, 2005
#

set(VBF_SOURCES 
  VArrayEvent.cpp
	VBankFileReader.cpp
	VDatum.cpp
	VException.cpp
	FPBasics.cpp
	FPIO.cpp
	FPIOCommon.c
	FPIOStats.cpp
	VSimulationData.cpp
	VSimulationHeader.cpp
	VKascadeSimulationData.cpp
	VCorsikaSimulationData.cpp
	VKascadeSimulationHeader.cpp
	VChiLASimulationData.cpp
	VChiLASimulationHeader.cpp
	Adler32.c
	VBankFileWriter.cpp
	VBitParser.cpp
	VGPSDecoder.cpp
	VRawDataExceptions.cpp
	VRawDataFileRead.cpp
	VRawEventParser.cpp
	FPFDWrite.cpp
	FPFDRead.cpp
	VBankFileStreamReader.cpp
	VBankFileBzip2Reader.cpp
	VEventOverflow.cpp
	VBankFileGzipReader.cpp
)

set(VBF_HEADERS 
  VArrayEvent.h
	VBank.h
	VBankBuilder.h
	VBankFileReader.h
	VBankName.h
	VConstants.h
	VDatum.h
	VException.h
	FPAdler.h
	FPBasics.h
	FPFDRead.h
	FPFDWrite.h
	FPIO.h
	FPIOCommon.h
	VPacket.h
	VSimulationHeader.h
	VSimulationData.h
	VKascadeSimulationHeader.h
	VKascadeSimulationData.h
	VCorsikaSimulationData.h
	VChiLASimulationHeader.h
	VChiLASimulationData.h
	VBFUtil.h
	Words.h
	Adler32.h
	VBankFileWriter.h
	VConfigMaskUtil.h
	VBitParser.h
	VGPSDecoder.h
	VRawDataExceptions.h
	VRawDataFileRead.h
	VRawEventData.h
	VRawEventParser.h
	VBankFileReaderBase.h
	VBankFileStreamReader.h
	VBankFileBzip2Reader.h
	VEventOverflow.h
	VWordParsing.h
	VEventType.h
	VBankFileGzipReader.h
	VSampleCompressor.h
	VSampleDecompressor.h
)

set(LIB_VERSION 1:1:0)

add_library(VBF_static STATIC ${VBF_SOURCES} ${VBF_HEADERS})
add_library(VBF SHARED ${VBF_SOURCES} ${VBF_HEADERS})