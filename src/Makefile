#
# Builds the rpi3rtos kernel image which the RPi video core loads from 
# the SDCard to memory location 0x00080000.
#

KERNEL_IMAGE = kernel8.img
COMPLIER = gcc
#COMPLILER = llvm

#######################################################################
# Targets
#######################################################################

all: kernel8.img

kernel8.img: $(COBJS) $(ASMOBJS)
	$(MAKE) -f Makefile.gcc -C ./hardware all
	$(MAKE) -f Makefile.gcc -C ./startup startup.img objdump
	$(MAKE) -f Makefile.gcc -C ./task0 task0.img objdump
	$(MAKE) -f Makefile.gcc -C ./taskN taskN.img objdump
	$(shell) cat \
	./startup/startup.img \
	./task0/task0.img \
	./taskN/taskN.img  >> $(KERNEL_IMAGE)

example: $(COBJS) $(ASMOBJS)
	$(MAKE) -f Makefile.gcc -C ./hardware all
	$(MAKE) -f Makefile.gcc -C ./startup startup.img objdump
	$(MAKE) -f Makefile.gcc -C ./task0 task0.img objdump
	$(MAKE) -f Makefile.gcc -C ./taskN taskN.img objdump
	$(MAKE) -f Makefile.gcc -C ./examples/task1 task1.img objdump
	$(shell) cat \
	./startup/startup.img \
	./task0/task0.img \
	./examples/task1/task1.img \
	./taskN/taskN.img  >> $(KERNEL_IMAGE)

clean:
	-rm -f ./$(KERNEL_IMAGE)
	$(MAKE) -f Makefile.gcc -C ./examples clean
	$(MAKE) -f Makefile.gcc -C ./hardware clean
	$(MAKE) -f Makefile.gcc -C ./startup clean
	$(MAKE) -f Makefile.gcc -C ./task0 clean
	$(MAKE) -f Makefile.gcc -C ./taskN clean
	$(MAKE) -C ./tasks clean

objdump:
	$(MAKE) -f Makefile.gcc -C ./startup objdump
	$(MAKE) -f Makefile.gcc -C ./task0 objdump
	$(MAKE) -f Makefile.gcc -C ./taskN objdump
	

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
DOCKER_EXEC_QEMU  = qemu-system-aarch64 -s -S -M raspi3 -kernel $(KERNEL_IMAGE)

qemu:
	$(DOCKER_CMD) $(DOCKER_ARG_CURDIR) $(CONTAINER_UTILS) \
	$(DOCKER_EXEC_QEMU) -serial stdio

