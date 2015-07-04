APPNAME=libc-sw.a
PLATFORM=m68000

CC=m68k-coff-gcc
AR=m68k-coff-ar
AS=m68k-coff-as
LD=m68k-coff-ld

BASEDIR=/opt/m68k/lib/gcc/m68k-coff/3.4.2
LIBGCC=$(BASEDIR)/$(PLATFORM)/libgcc.a

CFLAGS=-I$(BASEDIR)/include -Iinclude -I../os -c -g -Wall -O2 -$(PLATFORM)
AFLAGS=-$(PLATFORM)

CTYPE=ctype.c tolower.c toupper.c
STDIO=put.c putchar.c printf.c puts.c sprintf.c snprintf.c vprintf.c vsprintf.c vsnprintf.c
STDLIB=atoi.c atol.c calloc.c free.c malloc.c realloc.c strtol.c strtoul.c
STRING=memchr.c memcpy.c memset.c strcat.c strchr.c strcmp.c strcpy.c strdup.c strerror.c strncat.c strncpy.c strlen.c strstr.c
STRINGS=bcopy.c bzero.c

CSOURCES=assert.c $(CTYPE) errno.c $(STDIO) $(STDLIB) $(STRING) $(STRINGS)
ASMSOURCES=syscall.S
#LIBGCCOBJECTS=_divsi3.o _udivsi3.o
LIBGCCOBJECTS=

OBJECTS=$(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)

all: $(APPNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

.S.o:
	$(CC) $(CFLAGS) -o$@ $<

$(APPNAME): $(OBJECTS)
#	$(AR) x $(LIBGCC) $(LIBGCCOBJECTS)
	$(AR) rcs $(APPNAME) $(OBJECTS) $(LIBGCCOBJECTS)
#	rm -f $(LIBGCCOBJECTS)

clean:
	rm -f $(APPNAME) $(OBJECTS)

ctype.o: ctype.c include/ctype.h
tolower.o: tolower.c include/ctype.h
toupper.o: toupper.c include/ctype.h

atoi.o: atoi.c include/stdlib.h
atol.o: atol.c include/stdlib.h
calloc.o: calloc.c include/stdlib.h
free.o: free.c include/stdlib.h
malloc.o: malloc.c include/stdlib.h
realloc.o: realloc.c include/stdlib.h
strtol.o: strtol.c strtoul.o include/stdlib.h include/errno.h include/limits.h
strtoul.o: strtoul.c include/stdlib.h include/errno.h include/limits.h

errno.o: errno.c include/errno.h

sprintf.o: sprintf.c printf.o assert.o free.o malloc.o vsnprintf.o include/stdio.h
put.o: put.c include/stdio.h
puts.o: puts.c include/stdio.h
snprintf.o: snprintf.c vsnprintf.o include/stdio.h
sprintf.o: sprintf.c assert.o free.o malloc.o vsnprintf.o include/stdio.h
vprintf.o: vprintf.c assert.o free.o malloc.o include/stdio.h
vsnprintf.o: vsnprintf.c include/stdio.h
vsprintf.o: vsprintf.c assert.o free.o malloc.o vsnprintf.o include/stdio.h

memchr.o: memchr.c include/string.h
memcpy.o: memcpy.c include/strings.h
memset.o: memset.c include/string.h
strcat.o: strcat.c include/string.h include/stddef.h
strchr.o: strchr.c include/string.h include/stddef.h
strcmp.o: strcmp.c include/string.h include/stddef.h
strcpy.o: strcpy.c include/string.h include/stddef.h
strdup.o: strdup.c strlen.o malloc.o include/string.h include/stddef.h
strerror.o: strerror.c errno.o sprintf.o include/string.h include/stdlib.h
strlen.o: strlen.c include/string.h include/stddef.h
strncat.o: strncat.c include/string.h include/stddef.h
strncpy.o: strncpy.c include/string.h include/stddef.h
strstr.o: strstr.c include/string.h include/stddef.h

bcopy.o: bcopy.c include/strings.h
bzero.o: bzero.c include/strings.h

syscall.o: syscall.S include/unistd.h
