#
#Makefile for the rpi3rtos project. Uses gcc.
#

all:
	$(MAKE) -f Makefile.gcc -C ./src all

clean:
	$(MAKE) -f Makefile.gcc -C ./src clean

example:
	$(MAKE) -f Makefile.gcc -C ./src example

qemu:
	$(MAKE) -f Makefile.gcc -C ./src qemu

objdump:
	$(MAKE) -f Makefile.gcc -C ./src objdump
