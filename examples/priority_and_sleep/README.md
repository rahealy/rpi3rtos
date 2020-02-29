# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

### Priority And Sleep

This is an example consisting of three tasks which have increasing (task1 lowest, task3 highest) priority and FIFO scheduling. The kernel will run each task in priority order until it requests to be put to sleep. Task1 and Task2 will sleep for 1 tick. Task3 (highest priority) will sleep for 2 ticks. On wakeup tasks will resume in priority order:

Tick 1:
Task3 Work, Sleep 2 Ticks
Task2 Work, Sleep 1 Tick
Task1 Work, Sleep 1 Tick

Tick 2:
Task2 Work, Sleep 1 Tick
Task1 Work, Sleep 1 Tick

Tick 3:
Task3 Work, Sleep 2 Ticks
Task2 Work, Sleep 1 Tick
Task1 Work, Sleep 1 Tick

...
