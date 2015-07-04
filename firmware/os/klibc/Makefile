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

assert.o: assert.c assert.h ../include/types.h ../klibc/stdio.h
ctype.o: ctype.c ctype.h ../include/types.h
errno.o: errno.c errno.h ../include/types.h
printf.o: printf.c stdio.h ../memory/kmalloc.h ../include/limits.h ../include/types.h ../duart.h
stdio.o: stdio.c ../include/limits.h ../include/types.h ../duart.h
stdlib.o: stdlib.c stdlib.h ../include/limits.h ../include/types.h ../memory/kmalloc.h
string.o: string.c string.h ../include/limits.h ../include/types.h ../memory/kmalloc.h
strings.o: strings.c ../include/types.h

