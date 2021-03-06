# rpi3rtos
A Simple RTOS for the Raspberry Pi 3

A recent job application of mine was rejected because it did not contain the terms  "RTOS" and "C". As a result I decided to take on a personal daily challenge to write a simple RTOS in C for the Raspberry Pi 3. I've leveraged much of what I learned in my [rpi3fxproc](https://github.com/rahealy/rpi3fxproc) project and continue to see how much working code can be written in 1 day, 2 days, and so on...

### Current

**Saturday February 29, 2020 (Current)** <b>&ast;</b>

At this point I feel ready to call this a true multi-tasking operating system implementing FIFO and round-robin priority queues. February 10th was my first full day working on this project but I'm going to start counting days from February 9th. That makes 20 days from beginning to working pre-alpha. My informal goal for this point was 2 weeks (14 days.) I skipped a few days here and there to do other things with my life so I'm still pretty satisfied with the timeline.

Right now rpi3rtos is **not** a usable OS in the general sense. However, working proof of concept examples are provided. Please see the ~/rpi3rtos/examples/README.md for details.

<b>(&ast;)</b> Just a reminder that I'm actively looking for paid work either locally (Twin Cities, Minnesota, USA) or remotely. If you believe I could be of service to your organization or know of an organization that is looking for a dedicated employee committed to a lifetime of learning and serving others please feel free to contact me through [LinkedIn](https://www.linkedin.com/in/richardarthurhealy/). Thank you so much.

**What's next:**

* Finish MMU
  * MMU initialization routines are about 3/4 of the way done. 
* Special high priority tasks which provide interfaces to hardware 
  * UART, I2C, I2S, PWM, System Timers, ???.
* Kernel Logging 
  * De-clutter UART output.
  * Log levels (1-4)
* Performance Profiling
  * How long do things take?

### Older

**Wednesday February 26, 2020** Update covers several days. Task switching works, task suspend and sleep syscalls are in progress. Decided on a sorted linked list for task priority - currently the OS is limited to 8 tasks so btree or other data structure seems unnecessary. MMU is on hold - tasks need to behave on their own for now. Currently the kernel clock tick duration is one second to allow all the debugging messages to scroll by. Looking to get a pre-alpha release out later today.


**Saturday February 22, 2020 0752** - Update covers several days. Built GCC aarch64-elf cross compiler. QEMU doesn't support RPi3's system timer so switched over to local timer on core0. GCC always puts a `.got` section in the elf. Since this is silent it was necessary to put a `.got` section in the linker script so the linker script variables aligned to what is output.

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


