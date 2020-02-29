# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

## Examples
This directory contains working proof of concept examples.

### Priority And Sleep

This is an example consisting of three tasks which have different priorities and FIFO scheduling. The kernel will run each task in priority order until task requests to be put to sleep.

### Round Robin Example

This is an example consisting of three tasks which have requested the same priority and round-robin scheduling. The kernel will run each task sequentially for a slice of time.

### Building Examples

Currently, Makefiles are written to be compiled using an **aarch64-elf** targeted gcc cross compiler. Please see the **00_crosscompiler** section in bzt's raspi3-tutorial found [here](https://github.com/bztsrc/raspi3-tutorial) for details on how to build and install a gcc cross compiler.

Once the gcc cross compiler is built and installed install **docker** and **gdb_multiarch** for your platform.

In one console:

```
~/rpi3rtos$ cd examples/
~/rpi3rtos/examples$ make clean round_robin_qemu
...
qemu-system-aarch64 -s -S -M raspi3 -kernel kernel8.img -serial stdio
VNC server running on 127.0.0.1:5900
```

In another console:

```
~/rpi3rtos$ cd examples/round_robin/
~/rpi3rtos/examples/round_robin$ gdb-multiarch -x ./debug/gdbcommands.txt

```

When GDB opens:

```
...
--Type <RET> for more, q to quit, c to continue without paging--
Breakpoint 1 at 0x80000
(gdb) c
Continuing.

Thread 1 hit Breakpoint 1, 0x0000000000080000 in ?? ()
(gdb) c
Continuing.
```

Press Ctrl+C in GDB to break.

UART output from the Kernel and example tasks will be in the console running qemu.
