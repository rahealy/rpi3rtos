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

//
//task1.c
// Local references to globals from within the module reference the PIE 
// address.
//

#include "task.h"
#include "uart.h"
#include "kernel.h"

//*********************************************************************
// Task Specific Code
//*********************************************************************

//
//Global counter variable.
//
int g_counter = 0;

//
//Global counter variable.
//
static int s_counter = 0;

//
//Global pointer variable.
//
int *g_pointer = 0;

//
//Global pointer variable.
//
static int *s_pointer = 0;

//
//task1_main()
// Count up to 100000. Print '1'. Repeat.
//
void task1_main() {
    u64_t i;
    int *g_counter_ptr = &g_counter;
    int *s_counter_ptr = &s_counter;

    g_pointer = &g_counter;
    s_pointer = &s_counter;

    uart_puts("task1_main(): In task1.c\n");

    uart_puts("task1_main(): &g_counter = ");
    uart_u64hex_s((u64_t) &g_counter);
    uart_puts("\n");

    uart_puts("task1_main(): g_counter_ptr = ");
    uart_u64hex_s((u64_t) g_counter_ptr);
    uart_puts("\n");

    uart_puts("task1_main(): &s_counter = ");
    uart_u64hex_s((u64_t) &s_counter);
    uart_puts("\n");

    uart_puts("task1_main(): s_counter_ptr = ");
    uart_u64hex_s((u64_t) s_counter_ptr);
    uart_puts("\n");

    uart_puts("task1_main(): &g_pointer = ");
    uart_u64hex_s((u64_t) &g_pointer);
    uart_puts("\n");

    uart_puts("task1_main(): &s_pointer = ");
    uart_u64hex_s((u64_t) &s_pointer);
    uart_puts("\n");

    while(1) {
        for (i = 0; i < 100000; ++i) {
            ++g_counter;
        }
        uart_puts("1");
    }
}

//
//task1_init()
// Manadatory function.
//
void task1_init(u64_t arg) {
    task_suspend(KERNEL_TASK_FLAG_WAKEUP_POST_INIT);
    task1_main();
}

//
//task1_reset()
// Manadatory function.
//
void task1_reset(u64_t arg) {
    task_suspend(KERNEL_TASK_FLAG_WAKEUP_POST_RESET);
}

//*********************************************************************
// Mandatory OS Headers
//
// The operating system relies on the existence of headers linked at
// a specific offset in the task image to assist in loading and
// communicating with the task.
// 
//*********************************************************************

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
//tasklistitem{}
// List item for each task stored in the executable image. This gets
// loaded into first block of r/o memory in task's address space.
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
//task1_header{}
// Header located in 4kB aligned R/W memory shared by task and kernel.
//
volatile task_header task1_header
    __attribute__ ((section (".task_header"))) 
    __attribute__ ((__used__)) = {
    TASK_HEADER_MAGIC, 0,
    1, KERNEL_TASK_FLAG_QUEUE_ROUND_ROBIN,
    task1_init,
    task1_reset
};
