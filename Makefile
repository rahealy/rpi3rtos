#
#Makefile for the rpi3rtos project. Uses llvm.
#

CSRCS        = $(wildcard *.c)
ASMSRCS      = $(wildcard *.S)
COBJS        = $(CSRCS:.c=.o)
ASMOBJS      = $(ASMSRCS:.S=.o)
CTARGET      = aarch64-elf
LDTARGET     = aarch64elf
CFLAGS       = -Wall -O2 -ffreestanding -nostdinc -nostdlib -mcpu=cortex-a53+nosimd
KERNEL       = kernel8.elf
KERNEL_IMAGE = kernel8.img

#######################################################################
# Targets
#######################################################################


all: clean kernel8.img

kernel8.img: $(COBJS) $(ASMOBJS)
	ld.lld -m $(LDTARGET) -nostdlib $(ASMOBJS) $(COBJS) -T link.ld -o $(KERNEL)
	llvm-objcopy -O binary $(KERNEL) $(KERNEL_IMAGE)

%.o: %.c
	clang --target=$(CTARGET) $(CFLAGS) -c $< -o $@

%.o: %.S
	clang --target=$(CTARGET) $(CFLAGS) -c $< -o $@

clean:
	-rm -f ./$(KERNEL)
	-rm -f ./$(KERNEL_IMAGE)
	-rm -f *.o

#######################################################################
# Experimental Targets
#######################################################################

#
#Uses docker container from:
# https://github.com/rust-embedded/rust-raspi3-OS-tutorials
# Provided by Andre Richter <andre.o.richter@gmail.com>
#
CONTAINER_UTILS   = andrerichter/raspi3-utils

DOCKER_CMD        = docker run -p 1234:1234 -it --rm
DOCKER_ARG_CURDIR = -v $(shell pwd):/work -w /work
DOCKER_ARG_TTY    = --privileged -v /dev:/dev
DOCKER_EXEC_QEMU  = qemu-system-aarch64 -s -S -M raspi3 -kernel kernel8.img

qemu:
	$(DOCKER_CMD) $(DOCKER_ARG_CURDIR) $(CONTAINER_UTILS) \
	$(DOCKER_EXEC_QEMU) -serial stdio

objdump:
	llvm-objdump --disassemble $(KERNEL) > list.lst
