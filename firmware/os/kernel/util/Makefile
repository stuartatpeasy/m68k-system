include ../../global.mk

LIBNAME=../../libkutil.a

CSOURCES=bvec.c buffer.c checksum.c dump.c hash.c ktime.c numeric.c string.c

OBJECTS=$(CSOURCES:.c=.o)

all: $(LIBNAME)

STDDEPS=../include/defs.h ../include/types.h

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)

#
# Dependencies from this point on
#

bvec.o: \
	bvec.c bvec.h kutil.h $(STDDEPS) ../../klibc/errno.h ../../klibc/string.h \
	../include/memory/kmalloc.h

buffer.o: \
	buffer.c buffer.h $(STDDEPS)

checksum.o: \
	checksum.c $(STDDEPS)

dump.o: \
	dump.c kutil.h $(STDDEPS) ../../klibc/ctype.h ../include/memory/kmalloc.h

hash.o: \
	hash.c $(STDDEPS)

ktime.o: \
	ktime.c kutil.h $(STDDEPS) ../../kernel/include/limits.h

numeric.o: \
	numeric.c kutil.h $(STDDEPS)

string.o: \
	string.c kutil.h ../../klibc/ctype.h ../../klibc/string.h

