include ../global.mk

LIBNAME=../libc.a

CSOURCES=assert.c ctype.c errno.c printf.c stdio.c stdlib.c string.c strings.c

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

assert.o: \
	assert.c assert.h ../kernel/include/types.h ../klibc/stdio.h

ctype.o: \
	ctype.c ctype.h ../kernel/include/types.h

errno.o: \
	errno.c errno.h ../kernel/include/types.h

printf.o: \
	printf.c stdio.h string.h ../kernel/memory/kmalloc.h ../kernel/include/limits.h \
	../kernel/include/types.h ../kernel/include/platform.h

stdio.o: \
	stdio.c ../kernel/include/limits.h ../kernel/include/types.h ../kernel/include/platform.h

stdlib.o: \
	stdlib.c stdlib.h ../kernel/include/limits.h ../kernel/include/types.h \
	../kernel/memory/kmalloc.h

string.o: \
	string.c string.h ../kernel/include/limits.h ../kernel/include/types.h \
	../kernel/memory/kmalloc.h

strings.o: \
	strings.c ctype.h ../kernel/include/types.h
