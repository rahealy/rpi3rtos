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

#include "task.h"
#include "uart.h"

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

void task1_init(u64_t arg) {
    u64_t sptmp;
    asm volatile ( "mov %0, sp" : "=r"(sptmp) :: );
    
    uart_puts("rpi3rtos::task1_init(): Initializing task1 (sp = ");
    uart_u64hex_s(sptmp);
    uart_puts(") ...\n");
    uart_puts("rpi3rtos::task1_init(): Initialized task1. Suspending...\n");
    task_suspend();
    uart_puts("rpi3rtos::task1_init(): Woke from suspend. Working...\n");
    while(1) {
//        uart_puts("T1 ");
    }
}

void task1_reset(u64_t arg) {
    uart_puts("rpi3rtos::task1_reset(): Resetting task1...");
}

void task1_start(u64_t arg) {
    uart_puts("rpi3rtos::task1_start(): Start task1...\n");
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
//
//Header located in 4kB aligned R/W memory shared by task and kernel.
//
volatile task_header task1_header
    __attribute__ ((section (".task_header"))) 
    __attribute__ ((__used__)) = {
    TASK_HEADER_MAGIC, 0,
    task1_init,
    task1_reset,
    task1_start
};
