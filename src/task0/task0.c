/*
 * MIT License
 *
 * Copyright (c) 2020 Richard Healy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "irq.h"
#include "uart.h"
#include "task.h"
#include "timer.h"

//
//From linker script in order of appearance in memory.
//
extern int __task_ro_end;
extern int __task_rw_beg;
extern int __task_header_beg;
extern int __task_rw_end;
extern int __task_bss_beg;
extern int __task_bss_end;
extern int __task_bss_sz;

//
//From vectors.S
//
extern void __exception_vectors_start(void);

//
//Predefines
//
void start(void);
void reset(void);
void main(void);

//
//task0_panic()
// Infinite loop.
//
void task0_panic() {
    while(1) {
        asm("wfe":::);
    }
}

//
//Used by the loader to copy the task from the initial kernel image to 
//memory.
//
static task_list_item tasklistitem
    __attribute__ ((section (".task_list_item"))) 
    __attribute__ ((__used__)) = {
    TASK_LIST_ITEM_MAGIC,
    (u64_t) &__task_ro_end,
    (u64_t) &__task_rw_beg,
    (u64_t) &__task_rw_end,
    (u64_t) &__task_bss_beg,
    (u64_t) &__task_bss_end
};


//
//Header located in 4kB aligned R/W memory shared by task and kernel.
//
static task_header task0_header
    __attribute__ ((section (".task_header"))) 
    __attribute__ ((__used__)) = {
    TASK_HEADER_MAGIC,
    0, 0, 0, 0,
    start,
    reset,
    main
};

void start(void) {
    u64_t base = task_get_base_addr(0);

    uart_puts("rpi3rtos::task0_init(): Initializing kernel Task0...\n");

//Set exception handlers for EL1.
    uart_puts("rpi3rtos::task0_init(): Rebased __exception_vectors_start: ");
    uart_u64hex_s((u64_t) __exception_vectors_start + base);
    uart_puts("\n");
    asm volatile ("msr  vbar_el1, %0\n" :: "r"(__exception_vectors_start + base) :);
//__atomic_add_fetch((char *)0x80000000, (unsigned int)1, __ATOMIC_SEQ_CST);
//Initialize the timer.
    uart_puts("rpi3rtos::task0_init(): Timer test...\n");
    timer_init_core0(500);
    irq_enable();
    uart_puts("rpi3rtos::task0_init(): Timer test done.\n");
    task0_panic();
}

void reset(void) {
    uart_puts("rpi3rtos::task0_reset(): Resetting kernel Task0...");
}

void main() {
    uart_puts("rpi3rtos::task0_main(): Beginning kernel Task0...");
}
