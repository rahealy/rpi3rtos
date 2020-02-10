# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

**Tuesday February 4, 2020** - A recent job application rejection was keyword based for "RTOS" and "C". As a result I decided to take on a personal daily challenge to write a simple RTOS in C for the Raspberry Pi 3. I'll be leveraging much of what I learned in my [rpi3fxproc](https://github.com/rahealy/rpi3fxproc) project to see how much working code can be written in 1 day, 2 days, and so on...

**Monday February 10, 2020 (Current)** - This is more or less the first full day of work. After finishing up the [diaspora_client_example](https://github.com/rahealy/diaspora_client_example) I have time to focus on daily contributions to this project. Researching options for how to implement.


* Project Goals
  * Hardware
    * Global access to peripheral drivers by processes.
    * UART (PL011)
  * API
    * Message passing. 
      * FIFO in order by task priority?
  * MMU
    * Each task will run in its own block of memory and have a local stack.
    * Unity mapped memory split among tasks.
    * L1 and L2 cache increases performance but might alter predictability.
  * Timed slices of CPU
    * Each task will request one or more slices of time.
    * Round robin guarantees? Priority guarantees?
  * Scheduler
    * Task expected to maintain state of last successful slice.
    * Task expected to deal with slice timeout/retry.
  * IRQs
    * Handled by single high priority task.




