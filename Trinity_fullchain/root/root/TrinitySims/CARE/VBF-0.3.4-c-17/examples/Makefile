#
# Makefile for VBF example programs
# by Filip Pizlo, 2004
#
#

CXX=g++
CPPFLAGS=`vbfConfig --cppflags`
CXXFLAGS=`vbfConfig --cxxflags` -Wall

all: PrintEvents\
     PrintSpecificEvent\
     PrintCorsikaSimulationData\
     MakeBogusFile\
     MakeBogusFileWithCorsikaSimulationData\
     ProtoEventPrint\
     ProtoEventCount\
     PrintGPSTime\

clean:
	rm -f PrintEvents
	rm -f PrintSpecificEvent
	rm -f PrintMCEvent
	rm -f MakeBogusFile
	rm -f ProtoEventPrint
	rm -f ProtoEventCount
	rm -f PrintGPSTime
	rm -f *~ core* *.o

PrintEvents: PrintEvents.o
	$(CXX) -o PrintEvents \
		PrintEvents.o \
		`vbfConfig --ldflags --libs`

PrintSpecificEvent: PrintSpecificEvent.o
	$(CXX) -o PrintSpecificEvent \
		PrintSpecificEvent.o \
		`vbfConfig --ldflags --libs`

PrintCorsikaSimulationData: PrintCorsikaSimulationData.o
	$(CXX) -o PrintCorsikaSimulationData \
		PrintCorsikaSimulationData.o \
		`vbfConfig --ldflags --libs`

MakeBogusFile: MakeBogusFile.o
	$(CXX) -o MakeBogusFile \
		MakeBogusFile.o \
		`vbfConfig --ldflags --libs`

MakeBogusFileWithCorsikaSimulationData: MakeBogusFileWithCorsikaSimulationData.o
	$(CXX) -o MakeBogusFileWithCorsikaSimulationData \
		MakeBogusFileWithCorsikaSimulationData.o \
		`vbfConfig --ldflags --libs`

ProtoEventPrint: ProtoEventPrint.o
	$(CXX) -o ProtoEventPrint \
		ProtoEventPrint.o \
		`vbfConfig --ldflags --libs`

ProtoEventCount: ProtoEventCount.o
	$(CXX) -o ProtoEventCount \
		ProtoEventCount.o \
		`vbfConfig --ldflags --libs`

PrintGPSTime: PrintGPSTime.o
	$(CXX) -o PrintGPSTime \
		PrintGPSTime.o \
		`vbfConfig --ldflags --libs`

PrintEvents.o: PrintEvents.cpp

PrintSpecificEvent.o: PrintSpecificEvent.cpp

PrintCorsikaSimulationData.o: PrintCorsikaSimulationData.cpp

MakeBogusFile.o: MakeBogusFile.cpp

MakeBogusFileWithCorsikaSimulationData.o: MakeBogusFileWithCorsikaSimulationData.cpp

ProtoEventPrint.o: ProtoEventPrint.cpp

ProtoEventCount.o: ProtoEventCount.cpp

PrintGPSTime.o: PrintGPSTime.cpp

