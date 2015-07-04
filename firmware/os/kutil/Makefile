include ../global.mk

LIBNAME=../libkutil.a

CSOURCES=bvec.c ktime.c

OBJECTS=$(CSOURCES:.c=.o)

all: $(LIBNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)

#
# Dependencies from this point on
#

bvec.o: bvec.c bvec.h kutil.h ../include/defs.h ../include/types.h ../klibc/errno.h \
        ../klibc/string.h ../memory/kmalloc.h
ktime.o: ktime.c kutil.h ../include/defs.h ../include/types.h

