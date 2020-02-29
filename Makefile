#
#Makefile for the rpi3rtos project. Uses gcc.
#

all:
	$(MAKE) -f Makefile.gcc -C ./src all

clean:
	$(MAKE) -f Makefile.gcc -C ./src clean
	$(MAKE) -C ./debug clean

realclean: clean
	$(MAKE) -C ./examples clean

qemu:
	$(MAKE) -f Makefile.gcc -C ./src qemu

objdump:
	$(MAKE) -f Makefile.gcc -C ./src objdump
