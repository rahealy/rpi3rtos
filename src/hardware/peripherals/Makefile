#
# Compile to object files.
#

CSRCS        = $(wildcard */*/*.c)
ASMSRCS      = $(wildcard */*/*.S)
COBJS        = $(CSRCS:.c=.o)
ASMOBJS      = $(ASMSRCS:.S=.o)

#######################################################################
# Targets
#######################################################################


all: $(COBJS) $(ASMOBJS)

%.o: %.c
	clang $(CINCLUDES) --target=$(CTARGET) $(CFLAGS) -c $< -o $@

%.o: %.S
	clang --target=$(CTARGET) $(CFLAGS) -c $< -o $@

clean:
	-rm -f *.o
