# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

**Tuesday February 4, 2020** - A recent job application of mine was rejected because it did not contain the terms  "RTOS" and "C". As a result I decided to take on a personal daily challenge to write a simple RTOS in C for the Raspberry Pi 3. I'll be leveraging much of what I learned in my [rpi3fxproc](https://github.com/rahealy/rpi3fxproc) project to see how much working code can be written in 1 day, 2 days, and so on...

**Friday February 15, 2020 (Current)** - Got most of the MMU code written. Decided on memory layout and wrote startup loader. Wrote kernel (Task0) stub. Tried using autotools but couldn't make them do the right thing so backed out changes. Currently working on a linker issue that adds 10's of kB of 0x00 to the Task0 stub making ~8kB grow to ~40kB. Highly unexpected. Update: Adding something after the .bss segment in a linker script causes the linker to include the whole .bss section in the executable file. Will find another way.

**Wedneday February 12, 2020 0946** - Got a big chunk of the hardware groundwork (CPU init, exception table, UART) done yesterday. MMU and hardware timer will come today. Also, C compiler seems to be oblivious/not too aware of stack pointer manipulation in asm. It will probably take some experimenting to get the context switching right.

**Monday February 10, 2020** - This is more or less the first full day of work. After finishing up the [diaspora_client_example](https://github.com/rahealy/diaspora_client_example) I have time to focus on daily contributions to this project. Researching options for how to implement.


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


## Credits

### Software

This project contains code Copyright (c) 2018 Andre Richter <andre.o.richter@gmail.com> and bzt [https://github.com/bztsrc](https://github.com/bztsrc). All copyrights are respective of their owners.

Much of the RPi hardware specific code in this project is derived from information in the "Bare-metal and Operating System development tutorials in Rust on the Raspberry Pi 3" [https://github.com/rust-embedded/rust-raspi3-OS-tutorials](https://github.com/rust-embedded/rust-raspi3-OS-tutorials). This is a recommended resource for anyone interested in learning the specifics involved in solving problems in this particular domain.


