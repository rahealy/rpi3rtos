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

#include "uart.h"
#include "task.h"

extern int __bss_beg;
extern int __bss_sz;
extern int __task_end;
extern void __exception_vectors_start(void);

void start(void);
void reset(void);
void main(void);

static task_header taskheader
    __attribute__ ((section (".task_header"))) 
    __attribute__ ((__used__)) = {
    0, 0,
    start,
    reset,
    main
};

void start(void) {
    uart_puts("rpi3rtos::task0_init(): Initializing kernel Task0...");

//Zero the .bss segment.
    task_zero_bss((char *) &__bss_beg, (u64_t) &__bss_sz);

//Set exception handlers for EL1.
    asm volatile (
        "msr    vbar_el1, %0\n" :: "r"(__exception_vectors_start) :
    );
}

void reset(void) {
    uart_puts("rpi3rtos::task0_reset(): Resetting kernel Task0...");
}

void main() {
    uart_puts("rpi3rtos::task0_main(): Beginning kernel Task0...");
}

static task_list_item tasklistitem
    __attribute__ ((section (".task_list_item"))) 
    __attribute__ ((__used__)) = {
    TASK_LIST_ITEM_MAGIC, //Magic number.
    0,                    //This is task 0 so no next in list.
    (u64_t) &__task_end   //Size of task not including this header.
};
