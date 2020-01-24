CXX = g++
CXXFLAGS = -g -Wall -std=c++11 -O2 -fopenmp

INC = -I$(DYNINST_ROOT)/include
LIB = -L$(DYNINST_ROOT)/lib
DEP = -lparseAPI -linstructionAPI -lsymtabAPI -lcommon -lboost_system -lpthread -fopenmp -ltbbmalloc_proxy

all: BinFeat

SRC = feature.cpp\
      idiom.cpp\
      operand.cpp\
      lookup.cpp\
      BinFeat.cpp\
      graphlet.cpp\
      supergraph.cpp\
      colors.cpp\
      FeatureAnalyzer.cpp\
      IdiomAnalyzer.cpp\
      GraphletAnalyzer.cpp

OBJ = $(SRC:.cpp=.o)

%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $(INC) -o $@ $<

BinFeat: $(OBJ)
	$(CXX) $(LIB) -o $@ $^ -Wl,-rpath='$(DYNINST_ROOT)/lib' $(DEP) 

clean:
	rm -f core core.* *.core *.o BinFeat
