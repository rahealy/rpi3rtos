# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

## Null Task (taskN)

The null task is always the last task in the kernel image. It is used by the loader to determine the end of the task list in the kernel image. It is not loaded into memory.
