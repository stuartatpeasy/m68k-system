CC=g++
APPNAME=dfu
SOURCES=args.cpp dfu.cpp programmer.cpp serialport.cpp

OBJECTS=$(SOURCES:.cpp=.o)

CXXFLAGS=-c -Wall -O3 -std=c++11
LDFLAGS=

all: $(APPNAME)

$(APPNAME): $(OBJECTS)

.cpp.o:
	$(CC) $(CXXFLAGS) $< -o$@

clean:
	rm -f $(APPNAME) $(OBJECTS)

args.o: args.cpp args.h exception.h
dfu.o: dfu.cpp dfu.h args.h programmer.h exception.h
programmer.o: programmer.cpp programmer.h serialport.h exception.h
serialport.o: serialport.cpp serialport.h exception.h

