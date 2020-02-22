#
# Compile to object files.
#

#
# Builds the startup stub that gets loaded by the hardware to 0x00080000
#

CSRCS        = taskN.c
COBJS        = $(CSRCS:.c=.o)
CTARGET      = aarch64-elf
LDTARGET     = aarch64elf
CFLAGS       = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
CINCLUDES    = -I "." -I "../hardware" -I "../hardware/peripherals" -I "../tasks"

#######################################################################
# Targets
#######################################################################

all: taskN.img

taskN.img: $(COBJS) $(ASMOBJS)
	ld.lld -m $(LDTARGET) -nostdlib $(ASMOBJS) $(COBJS) -T ./link.ld -o taskN.elf
	llvm-objcopy -O binary taskN.elf taskN.img

%.o: %.c
	clang $(CINCLUDES) --target=$(CTARGET) $(CFLAGS) -c $< -o $@

clean:
	-rm -f taskN.elf
	-rm -f taskN.img
	-rm -f *.o

objdump:
	llvm-objdump --all-headers --full-contents --disassemble taskN.elf > ../../debug/taskN.lst
