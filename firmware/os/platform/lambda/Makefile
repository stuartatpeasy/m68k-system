SRC_ROOT=../..

include $(SRC_ROOT)/global.mk

LIBNAME := $(SRC_ROOT)/libplatform.a

CSOURCES := device.c dfu.c lambda.c

ASMSOURCES := startup.S

OBJECTS := $(CSOURCES:.c=.o) $(ASMSOURCES:.S=.o)

DEPDIR := .deps
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

COMPILE.S = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c

%.o : %.c $(DEPDIR)/%.d
	$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -o$@ $<

%.o : %.S $(DEPDIR)/%.d
	$(CC) -o$@ $<
#	$(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -o$@ $<

$(DEPDIR)/%.d:
	@mkdir -p $(dir $@)

.PRECIOUS: $(DEPDIR)/%.d

all: $(LIBNAME)

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(CSOURCES)))

$(LIBNAME): $(OBJECTS)
	$(AR) rcs $(LIBNAME) $(OBJECTS)

clean:
	rm -f $(LIBNAME) $(OBJECTS)
	rm -rf $(DEPDIR)
