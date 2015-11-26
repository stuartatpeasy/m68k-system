include ../global.mk

LIBNAME=../libmonitor.a

CSOURCES=commands.c disasm.c env.o history.o monitor.o readline.o srec.o

CFLAGS+=-I..

OBJECTS=$(CSOURCES:.c=.o)

all: $(LIBNAME)

.c.o:
	$(CC) $(CFLAGS) -o$@ $<

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)

commands.o: \
	commands.c monitor.h disasm.o srec.o ../kernel/memory/kmalloc.h ../kernel/memory/kmalloc.c \
	../kernel/util/kutil.h

disasm.o: \
	disasm.c disasm.h ../kernel/util/kutil.h

env.o: \
	env.c env.h

history.o: \
	history.c history.h ../kernel/include/defs.h ../kernel/include/types.h

monitor.o: \
	monitor.c monitor.h commands.o disasm.h history.h readline.h srec.h ../kernel/device/nvram.h \
	../kernel/fs/mount.h ../kernel/include/defs.h ../kernel/include/version.h \
	../kernel/util/kutil.h ../kernel/memory/kmalloc.h ../kernel/memory/slab.h

readline.o: \
	readline.c readline.h ../kernel/platform.h

srec.o: \
	srec.c srec.h readline.o ../kernel/util/kutil.h
