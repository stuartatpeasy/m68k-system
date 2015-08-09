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
	commands.c monitor.h disasm.o srec.o ../memory/kmalloc.h ../memory/kmalloc.c ../dfu.c ../dfu.h \
	../kutil/kutil.h

disasm.o: \
	disasm.c disasm.h ../kutil/kutil.h ../libkutil.a

env.o: \
	env.c env.h

history.o: \
	history.c history.h ../include/defs.h

monitor.o: \
	monitor.c monitor.h commands.o disasm.h history.h readline.h srec.h ../device/bbram.h \
	../device/ds17485.h ../include/defs.h ../include/version.h ../kutil/kutil.h \
	../memory/kmalloc.h ../memory/ramdetect.h ../memory/slab.h

readline.o: \
	readline.c readline.h ../device/duart.h

srec.o: \
	srec.c srec.h readline.o ../kutil/kutil.h ../libkutil.a
