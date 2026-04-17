
# Makefile for IFHB
#
#
# 
#By: Oscar F Romero Matamala

#SourceDir
SRC_DIR := src
OBJ_DIR := obj
PWD := $(shell pwd)
PROJECT := IFHB


BIN_DIR := .
EXE := $(BIN_DIR)/IStayOnGround

ARCH := $(shell uname)

# linux flags
ifeq ($(ARCH),Linux)
CXX           = g++ 
CXXFLAGS      =  -g -O3 -Wall -ggdb3 -fPIC -fno-strict-aliasing  -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE_SOURCE -D_LARGEFILE64_SOURCE
LD            = g++
LDFLAGS       = -g
SOFLAGS       = -shared
endif

# Apple OS X flags
ifeq ($(ARCH),Darwin)
CXX           = g++ 
CXXFLAGS      = -g -O3 -Wall -fPIC  -fno-strict-aliasing
LD            = g++
LDFLAGS       = -O
SOFLAGS       = -shared
endif

OutPutOpt     = -o


# Root

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTLIBS     := $(shell root-config --libs)
ROOTGLIBS    := $(shell root-config --glibs)
ROOTLDFLAGS  := $(shell root-config --ldflags)
ROOTLIBDIR   := $(shell root-config --libdir)

CLLFLAGS += $(ROOTLDFLAGS)

CXXFLAGS     += $(ROOTCFLAGS)
LIBS          = $(ROOTGLIBS)
LIBS         += -lMinuit -lMathMore -lEve -lGeom -L./ -L./include 


INCLUDEFLAGS  = -Idict -Iinclude -I./inc/ -I./
CXXFLAGS     += $(INCLUDEFLAGS)

ALLFLAGS = $(CXXFLAGS) $(CPPFLAGS) -Wall

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
INC := $(SRC:$(SRC_DIR)/%.cpp=$(BIN_DIR)/include/%.h)
DEP := $(SRC:$(SRC_DIR)/%.cpp,%.d,$(SOURCES))



CP  := cp

#Recipe

.PHONY: all
all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR) 
	$(LD) $(OBJ) $(CXXFLAGS) $(LIBS) $(LDFLAGS) $(OutPutOpt) $@ -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
	@echo "$@ Compilation done"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(LD) $(CXXFLAGS) -MMD -MP -c $< -o $@ 

$(BIN_DIR)/$(OBJ_DIR):
		mkdir -p $@

-include $(OBJ:.o=.d)
-include $(DEP)


docs: $(DOCS_DIR)/$(PROJECT).html

$(DOCS_DIR)/$(PROJECT).html: $(EXE) $(LIB)
	@echo Building documentation for ExACT
	@doxygen ./cfg/doxy.cfg

clean:
	rm -r $(OBJ_DIR)
	rm $(EXE)

.SUFFIXES: .o

#$(BIN_DIR)/IStayOnGround: $(OBJ) | $(BIN_DIR)
#	$(LD) $(OBJ) $(CXXFLAGS) $(LIBS) $(LDFLAGS) $(OutPutOpt) $@ -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
#	@echo "$@ Compilation done"