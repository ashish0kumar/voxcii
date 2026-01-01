CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall
LIBS = -lncurses

SRC = src/*.cpp
BIN = voxcii

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN) $(LIBS)

clean:
	rm -f $(BIN)
