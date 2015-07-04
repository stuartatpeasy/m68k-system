CC=g++
APPNAME=flashtool
SOURCES=args.C flashtool.C programmer.C serialport.C

OBJECTS=$(SOURCES:.C=.o)

CXXFLAGS=-c -Wall -O3 -std=c++0x
LDFLAGS=

all: $(APPNAME)

$(APPNAME): $(OBJECTS)

.C.o:
	$(CC) $(CXXFLAGS) $< -o$@

clean:
	rm -f $(APPNAME) $(OBJECTS)

args.o: args.C args.h exception.h
flashtool.o: flashtool.C flashtool.h args.h programmer.h exception.h
programmer.o: programmer.C programmer.h serialport.h exception.h
serialport.o: serialport.C serialport.h exception.h

