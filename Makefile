CXX = g++
CXXFLAGS = -g -Wall -std=c++11

all: lizards lizardsUni

lizards: lizards.cpp
	$(CXX) $(CXXFLAGS) lizards.cpp -o lizards -pthread

lizardsUni: lizardsUni.cpp
	$(CXX) $(CXXFLAGS) lizardsUni.cpp -o lizardsUni -pthread

clean:
	rm -f lizards lizardsUni
	rm -rf lizards.dSYM lizardsUni.dSYM

.PHONY: all clean
