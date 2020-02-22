# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

**Tuesday February 4, 2020** - A recent job application of mine was rejected because it did not contain the terms  "RTOS" and "C". As a result I decided to take on a personal daily challenge to write a simple RTOS in C for the Raspberry Pi 3. I'll be leveraging much of what I learned in my [rpi3fxproc](https://github.com/rahealy/rpi3fxproc) project to see how much working code can be written in 1 day, 2 days, and so on...

**Saturday February 22, 2020 0752 (Current)** - Update covers several days. Built GCC aarch64-elf cross compiler. QEMU doesn't support RPi3's system timer so switched over to local timer on core0. GCC always puts a `.got` section in the elf. Since this is silent it was necessary to put a `.got` section in the linker script so the linker script variables aligned to what is output.

Below is output from the startup loader and Task0 (kernel)
```
rpi3rtos::startup(): Found startup list header at 0x8C000.
rpi3rtos::startup(): Loading tasks from list at 0x8C008...
rpi3rtos::startup_load_task_list(): Found list item at 0x8C008:
rpi3rtos::startup_load_task_list(): ro_end - 0x1BA2
rpi3rtos::startup_load_task_list(): rw_beg - 0x2000
rpi3rtos::startup_load_task_list(): rw_end - 0x2078
rpi3rtos::startup_load_task_list(): bss_beg - 0x2080
rpi3rtos::startup_load_task_list(): bss_end - 0xC000
rpi3rtos::startup_load_task_list(): Seeking next list item at 0x8E080...
rpi3rtos::startup_load_task_list(): Found list item at 0x8E080
rpi3rtos::startup_load_task_list(): End of list. Begin loading.
rpi3rtos::startup_load_task_list(): Loading task 0x0 image.
rpi3rtos::startup_load_task_list(): Loading read-only segment
rpi3rtos::startup_load_task_list(): 0x8C008->0x200000(0x1BA2 Bytes)
rpi3rtos::startup_load_task_list(): Loading read-write segment
rpi3rtos::startup_load_task_list(): 0x8E008->0x202000(0x78 Bytes)
rpi3rtos::task_header_rebase(): Re-basing task 0x0 header to 0x200000
rpi3rtos::task_header_rebase(): 0x40->0x200040
rpi3rtos::task_header_rebase(): 0x30->0x200030
rpi3rtos::task_header_rebase(): 0xB8->0x2000B8
rpi3rtos::task_bss_zero(): 0x202080-0x20C000
rpi3rtos::startup(): Calling task0->start() (0x202000 0x200040)
rpi3rtos::task0_init(): Initializing kernel Task0...
rpi3rtos::task0_init(): Rebased __exception_vectors_start: 0x201000
rpi3rtos::task0_init(): Timer test...
rpi3rtos::task0_init(): Timer test done.
current_elx_irq(): Timer tick!
current_elx_irq(): Timer tick!
```

**Monday February 17, 2020 0312** - Loading tasks from kernel image works. Calling Task0 (kernel) start() function works. Beginning Task0 kernel code. First up - timer interrupts for context switches.

**Friday February 15, 2020** - Got most of the MMU code written. Decided on memory layout and wrote startup loader. Wrote kernel (Task0) stub. Tried using autotools but couldn't make them do the right thing so backed out changes. Currently working on a linker issue that adds 10's of kB of 0x00 to the Task0 stub making ~8kB grow to ~40kB. Highly unexpected. Update: Adding something after the .bss segment in a linker script causes the linker to include the whole .bss section in the executable file. Will find another way.

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


