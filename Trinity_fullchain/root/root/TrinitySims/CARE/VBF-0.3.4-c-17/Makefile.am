# $Header: /home/cvsuser/VERITAS/software/common/VBF/Makefile.am,v 1.6 2015/03/23 18:12:20 sembrosk Exp $
#
# Automake file for VBF
# by Filip Pizlo, 2004, 2005
# updated Juniper Foote, 2022-2024
#

SUBDIRS = VBF vbfTools scripts

EXTRA_DIST = examples/MakeBogusFile.cpp examples/MakeBogusFileWithCorsikaSimulationData.cpp examples/Makefile examples/PrintCorsikaSimulationData.cpp examples/PrintEvents.cpp examples/PrintGPSTime.cpp examples/PrintSpecificEvent.cpp examples/ProtoEventCount.cpp examples/ProtoEventPrint.cpp examples/README

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = VBF.pc
ACLOCAL_AMFLAGS=-I m4

# extra clean target for Sun CC compiler and generated distribution tarball
clean-local:
	-rm -rf SunWS_cache
	-rm -rf *.tar.gz
