include ../global.mk

LIBNAME=../libc.a

CSOURCES=assert.c ctype.c errno.c printf.c stdio.c stdlib.c string.c strings.c

OBJECTS=$(CSOURCES:.c=.o)

KERN_INC=../kernel/include

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
	assert.c include/assert.h $(KERN_INC)/types.h include/stdio.h

ctype.o: \
	ctype.c include/ctype.h $(KERN_INC)/types.h

errno.o: \
	errno.c include/errno.h $(KERN_INC)/types.h

printf.o: \
	printf.c include/stdio.h include/string.h $(KERN_INC)/memory/kmalloc.h $(KERN_INC)/limits.h \
	$(KERN_INC)/types.h $(KERN_INC)/platform.h

stdio.o: \
	stdio.c $(KERN_INC)/limits.h $(KERN_INC)/types.h $(KERN_INC)/platform.h

stdlib.o: \
	stdlib.c include/stdlib.h $(KERN_INC)/limits.h $(KERN_INC)/types.h $(KERN_INC)/memory/kmalloc.h

string.o: \
	string.c include/string.h $(KERN_INC)/limits.h $(KERN_INC)/types.h $(KERN_INC)/memory/kmalloc.h

strings.o: \
	strings.c include/ctype.h $(KERN_INC)/types.h
