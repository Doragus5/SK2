CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -Wshadow -pedantic -Werror
LDFLAGS = -lstdc++fs
OBJECTS = *.o

.PHONY: all clean

all: client

serwer: $(OBJECTS) 
    $(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c $< -o $@

clean:
    rm *.o serwer