# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

Startup is the first code run in the RTOS. It boots, puts the CPU into EL1, loads the tasks from the startup image into memory and jumps to task0::main().
