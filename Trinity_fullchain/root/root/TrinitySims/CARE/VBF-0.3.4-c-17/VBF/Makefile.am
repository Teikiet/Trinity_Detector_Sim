# $Header: /home/cvsuser/VERITAS/software/common/VBF/VBF/Makefile.am,v 1.3 2015/02/24 10:03:37 fleish Exp $
#
# Automake file for VBF
# by Filip Pizlo, 2004, 2005
#
#

VBFdir = $(includedir)/VBF

VBF_SOURCES = VArrayEvent.cpp\
	VBankFileReader.cpp\
	VDatum.cpp\
	VException.cpp\
	FPBasics.cpp\
	FPIO.cpp\
	FPIOCommon.c\
	FPIOStats.cpp\
	VSimulationData.cpp\
	VSimulationHeader.cpp\
	VKascadeSimulationData.cpp\
	VCorsikaSimulationData.cpp\
	VKascadeSimulationHeader.cpp\
	VChiLASimulationData.cpp\
	VChiLASimulationHeader.cpp\
	Adler32.c\
	VBankFileWriter.cpp\
	VBitParser.cpp\
	VGPSDecoder.cpp\
	VRawDataExceptions.cpp\
	VRawDataFileRead.cpp\
	VRawEventParser.cpp\
	FPFDWrite.cpp\
	FPFDRead.cpp\
	VBankFileStreamReader.cpp\
	VBankFileBzip2Reader.cpp\
	VEventOverflow.cpp\
	VBankFileGzipReader.cpp

VBF_HEADERS = VArrayEvent.h\
	VBank.h\
	VBankBuilder.h\
	VBankFileReader.h\
	VBankName.h\
	VConstants.h\
	VDatum.h\
	VException.h\
	FPAdler.h\
	FPBasics.h\
	FPFDRead.h\
	FPFDWrite.h\
	FPIO.h\
	FPIOCommon.h\
	VPacket.h\
	VSimulationHeader.h\
	VSimulationData.h\
	VKascadeSimulationHeader.h\
	VKascadeSimulationData.h\
	VCorsikaSimulationData.h\
	VChiLASimulationHeader.h\
	VChiLASimulationData.h\
	VBFUtil.h\
	Words.h\
	Adler32.h\
	VBankFileWriter.h\
	VConfigMaskUtil.h\
	VBitParser.h\
	VGPSDecoder.h\
	VRawDataExceptions.h\
	VRawDataFileRead.h\
	VRawEventData.h\
	VRawEventParser.h\
	VBankFileReaderBase.h\
	VBankFileStreamReader.h\
	VBankFileBzip2Reader.h\
	VEventOverflow.h\
	VWordParsing.h\
	VEventType.h\
	VBankFileGzipReader.h\
	VSampleCompressor.h\
	VSampleDecompressor.h

LIB_VERSION = 1:1:0

library_includedir=$(includedir)/VBF
library_include_HEADERS = $(VBF_HEADERS)

AM_CPPFLAGS = -I$(top_srcdir)

lib_LTLIBRARIES = libVBF.la
libVBF_la_SOURCES = $(VBF_HEADERS) $(VBF_SOURCES)
libVBF_la_LDFLAGS = -version-info $(LIB_VERSION)

