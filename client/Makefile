CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -Wshadow -pedantic -Werror
LDFLAGS = -lstdc++fs
OBJECTS = *.o

.PHONY: all clean

all: klient

klient: client.o err.o 
	$(CC) -o $@ $^ $(LDFLAGS)

client.o: client.cpp 
	$(CC) $(CFLAGS) -c $<

err.o: err.cpp 
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o serwer