# Makefile for the lambda platform port of ayumos
#
# Lambda is a single-board computer with a MC68010 CPU, dual RS232 ports (via a MC68681), an
# ATA interface, a DS17485 RTC and a buzzer.  It has four expansion slots, and supports up to 8MB
# of static RAM.  The bottom 256KB of installed RAM is restricted to supervisor-mode accesses, as
# are the expansion slots.
#
# Stuart Wallace <stuartw@atom.net>, 2011-2017.
#
SRC_ROOT=../..

include $(SRC_ROOT)/global.mk

CFLAGS += -I$(SRC_ROOT)

LIBNAME := $(SRC_ROOT)/libplatform.a

CSOURCES := device.c dfu.c lambda.c

ASMSOURCES := startup.S

DEPDIR := .deps
OBJDIR := .obj

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

#OBJECTS := $(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)
OBJECTS := $(addprefix $(OBJDIR)/,$(patsubst %.c,%.o,$(CSOURCES)) $(patsubst %.S,%.o,$(ASMSOURCES)))

#COMPILE.S = $(CC) $(DEPFLAGS) $(CFLAGS) $(TARGET_ARCH) -c

$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "(TARGETCC) $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(TARGET_ARCH) -c -o$@ $<

$(OBJDIR)/%.o : %.S $(DEPDIR)/%.d
	@mkdir -p $(dir $@)
	@echo "(TARGETAS) $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(TARGET_ARCH) -c -o$@ $<

$(DEPDIR)/%.d:
	@mkdir -p $(dir $@)

.PRECIOUS: $(DEPDIR)/%.d
.PRECIOUS: $(OBJDIR)/%.o

all: $(LIBNAME)

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(CSOURCES)))

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)
	rm -rf $(DEPDIR) $(OBJDIR)
