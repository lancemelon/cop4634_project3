# Makefile for Hungry Lizard Crossing

CXX = g++
CXXFLAGS = -g -Wall -std=c++11 -pthread

all: lizards lizardsUni

lizards: lizards.cpp
	$(CXX) $(CXXFLAGS) lizards.cpp -o lizards

lizardsUni: lizardsUni.cpp
	$(CXX) $(CXXFLAGS) lizardsUni.cpp -o lizardsUni

clean:
	rm -f lizards lizardsUni