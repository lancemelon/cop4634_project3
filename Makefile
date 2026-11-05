CXX = g++
CXXFLAGS = -g -Wall -std=c++11

all: lizard

lizard: lizard.cpp
	$(CXX) $(CXXFLAGS) lizard.cpp -o lizard -pthread

clean:
	rm -f lizard
	rm -rf lizard.dSYM

.PHONY: all clean
