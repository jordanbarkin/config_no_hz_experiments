# the compiler: gcc for C program, define as g++ for C++
CC = g++

# compiler flags:
CFLAGS  = -pthread -O0

all: calibrate multithreaded

calibrate: calibrate.cpp
	$(CC) $(CFLAGS) -o calibrate.out calibrate.cpp

multithreaded: multithreaded.cpp
	$(CC) $(CFLAGS) -o multithreaded.out multithreaded.cpp

clean:
	$(RM) $(TARGET)
