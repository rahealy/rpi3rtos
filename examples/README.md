# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

### Priority And Sleep

This is an example consisting of three tasks which have different priorities and FIFO scheduling. The kernel will run each task in priority order until task requests to be put to sleep.

### Round Robin Example

This is an example consisting of three tasks which have requested the same priority and round-robin scheduling. The kernel will run each task sequentially for a slice of time.
