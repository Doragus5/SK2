CC = g++
CFLAGS = -Wall -Wextra -O2 -std=c++17
LDFLAGS= -lstdc++fs 

.PHONY: all clean

all: serwer

serwer: main.o server.o err.o misc_structs.o parser.o
	$(CC) -o $@ $^ $(LDFLAGS)

main.o: main.cpp 
	$(CC) $(CFLAGS) -c $<

server.o: server.cpp 
	$(CC) $(CFLAGS) -c $<

err.o: err.cpp 
	$(CC) $(CFLAGS) -c $<

parser.o: parser.cpp 
	$(CC) $(CFLAGS) -c $<

misc_structs.o: misc_structs.cpp  
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o serwer
