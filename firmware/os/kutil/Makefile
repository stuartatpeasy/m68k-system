include ../global.mk

LIBNAME=../libkutil.a

CSOURCES=bcopy.c bvec.c ctype.c kbzero.c kmemcpy.c kprintf.c kputchar.c kput.c kputs.c      \
         kstrcat.c kstrcmp.c kstrcpy.c kstrdup.c kstrlen.c kstrncmp.c kstrncpy.c kstrstr.c  \
		 kstrtoul.c ktime.c strerror.c

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

bcopy.o: bcopy.c kutil.h ../include/types.h
bvec.o: bvec.c bvec.h kutil.h ../include/errno.h ../include/defs.h ../include/types.h ../memory/kmalloc.h
ctype.o: ctype.c ../include/ctype.h ../include/types.h
kbzero.o: kbzero.c kutil.h ../include/types.h
kmemcpy.o: kmemcpy.c kutil.h ../include/types.h
kprintf.o: kprintf.c kutil.h ../include/limits.h ../memory/kmalloc.h ../duart.h
kputchar.o: kputchar.c kutil.h ../include/types.h ../duart.h
kput.o: kput.c kutil.h ../include/debug.h ../duart.h
kputs.o: kputs.c kutil.h ../include/debug.h ../duart.h
kstrcat.o: kstrcat.c kutil.h kstrlen.c kstrcpy.c ../include/types.h
kstrcmp.o: kstrcmp.c kutil.h ../include/types.h
kstrcpy.o: kstrcpy.c kutil.h ../include/types.h
kstrlen.o: kstrlen.c kutil.h ../include/types.h
kstrncmp.o: kstrncmp.c kutil.h ../include/types.h
kstrncpy.o: kstrncpy.c kutil.h ../include/types.h
kstrstr.o: kstrstr.c kutil.h ../include/types.h
kstrtoul.o: kstrtoul.c kutil.h ../include/types.h ../include/limits.h
ktime.o: ktime.c kutil.h ../include/types.h
strerror.o: strerror.c kutil.h ../include/errno.h ../include/types.h

