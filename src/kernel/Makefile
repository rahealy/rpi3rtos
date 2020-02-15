#
# Compile to object files.
#

#
# Builds the startup stub that gets loaded by the hardware to 0x00080000
#

CSRCS        = $(wildcard *.c) 
CSRCS       += $(wildcard ../hardware/peripherals/*.c) 
CSRCS       += $(wildcard ../tasks/*.c)

ASMSRCS      = $(wildcard *.S)
ASMSRCS     += $(wildcard ../hardware/peripherals/*.S)
ASMSRCS     += $(wildcard ../tasks/*.S)

COBJS        = $(CSRCS:.c=.o)
ASMOBJS      = $(ASMSRCS:.S=.o)

CTARGET      = aarch64-elf
LDTARGET     = aarch64elf
CFLAGS       = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
CINCLUDES    = -I "." -I "../hardware" -I "../hardware/peripherals" -I "../tasks"

#######################################################################
# Targets
#######################################################################

all: task0.img

task0.img: $(COBJS) $(ASMOBJS)
	ld.lld -m $(LDTARGET) -nostdlib $(ASMOBJS) $(COBJS) -T ./link.ld -o task0.elf
	llvm-objcopy -O binary task0.elf task0.img

%.o: %.c
	clang $(CINCLUDES) --target=$(CTARGET) $(CFLAGS) -c $< -o $@

%.o: %.S
	clang --target=$(CTARGET) $(CFLAGS) -c $< -o $@

clean:
	-rm -f task0.elf
	-rm -f task0.img
	-rm -f *.o

objdump:
	llvm-objdump --all-headers --full-contents --disassemble task0.elf > ../../debug/task0.lst
