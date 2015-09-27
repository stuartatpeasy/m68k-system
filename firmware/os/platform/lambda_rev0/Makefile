SRC_ROOT=../..
OS_INCDIR=$(SRC_ROOT)/include

include $(SRC_ROOT)/global.mk

LIBNAME=$(SRC_ROOT)/libplatform.a

CSOURCES=device.c lambda.c mc68681.c

OBJECTS=$(CSOURCES:.c=.o)

STDDEPS=../spec.h $(OS_INCDIR)/defs.h $(OS_INCDIR)/error.h $(OS_INCDIR)/types.h

all: $(LIBNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)

device.o: \
	$(STDDEPS) device.c device.h

lambda.o: \
	$(STDDEPS) lambda.c lambda.h

mc68681.o: \
	$(STDDEPS) mc68681.c mc68681.h

