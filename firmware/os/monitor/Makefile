include ../global.mk

LIBNAME=../libmonitor.a

CSOURCES=commands.c disasm.c env.o history.o monitor.o readline.o srec.o

CFLAGS+=-I..

OBJECTS=$(CSOURCES:.c=.o)

KERN_INC=../kernel/include

all: $(LIBNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)

commands.o: \
	commands.c monitor.h disasm.o srec.o $(KERN_INC)/memory/kmalloc.h ../kernel/util/kutil.h

disasm.o: \
	disasm.c disasm.h ../kernel/util/kutil.h

env.o: \
	env.c env.h

history.o: \
	history.c history.h $(KERN_INC)/defs.h $(KERN_INC)/types.h

monitor.o: \
	monitor.c monitor.h commands.o disasm.h history.h readline.h srec.h ../kernel/device/nvram.h \
	../kernel/fs/mount.h $(KERN_INC)/defs.h $(KERN_INC)/version.h \
	../kernel/util/kutil.h $(KERN_INC)/memory/kmalloc.h $(KERN_INC)/memory/slab.h

readline.o: \
	readline.c readline.h $(KERN_INC)/platform.h

srec.o: \
	srec.c srec.h readline.o ../kernel/util/kutil.h
