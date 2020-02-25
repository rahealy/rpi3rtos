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

#include "kernel.h"
#include "uart.h"
#include "mmu.h"

extern volatile u64_t __task_context_save(u64_t);

//
//kernel_panic()
// Infinite loop.
//
void kernel_panic() {
    while(1) {
        asm("wfe":::);
    }
}

int kernel_init(volatile kernel *k, u64_t num_tasks) {
    u64_t i;

    uart_puts("rpi3rtos::kernel_init(): Initializing kernel (");
    uart_u64hex_s((u64_t) k);
    uart_puts(") ");
    uart_u64hex_s(num_tasks);
    uart_puts(" tasks...\n");

    if (num_tasks > KERNEL_TASKS) {
        uart_puts("rpi3rtos::kernel_init(): Number of tasks exceeds maximum allowed. Panic.\n");
        kernel_panic();
    }

    for (i = 0; i < num_tasks; ++i) {
        k->tasks[i].header = task_get_header(i); 
        k->tasks[i].priority = 0;
        k->tasks[i].sp = task_get_base_addr(i);
    }

    k->num_tasks = num_tasks;
    k->task = 1;

    uart_puts("rpi3rtos::kernel_init(): Done.\n");

    return 0;
}

u64_t kernel_tasks_get_ptr_sp(volatile kernel *k) {
    return ((u64_t) &k->tasks[k->task].sp) + task_get_base_addr(k->task);
}

void kernel_tasks_init(volatile kernel *k) {
    uart_puts("rpi3rtos::kernel_tasks_init(): Initializing tasks ");
    uart_u64hex_s(1);
    uart_puts("-");
    uart_u64hex_s(k->num_tasks);
    uart_puts("...\n");

    for (k->task = 1; k->task < k->num_tasks; ++k->task) {
        __task_context_save_and_branch (
            &k->tasks[0].sp,                        //Kernel context stack pointer saved here.
            k->tasks[k->task].sp,                   //Context set to task stack pointer. 
            (u64_t) k->tasks[k->task].header->init  //Function called in new context.
        );
        uart_puts("rpi3rtos::kernel_tasks_init(): Task suspended. ");
        uart_puts("sp = ");
        uart_u64hex_s(k->tasks[k->task].sp);
        uart_puts(" sp ptr = ");
        uart_u64hex_s(kernel_tasks_get_ptr_sp(k)); // (u64_t) &k->tasks[k->task].sp);
        uart_puts("\n");
        //Task init() should call task_suspend() which will bring us back here.
    }

    k->task = 1;
    uart_puts("rpi3rtos::kernel_tasks_init(): Done.\n");
}

void kernel_main(volatile kernel *k) {
    uart_puts("rpi3rtos::kernel_main(): Entering main kernel loop.\n");

    while(1) {
        uart_puts("rpi3rtos::kernel_main(): Switching to task ");
        uart_u64hex_s(k->task);
        uart_puts(".\n");

//Switch to currently running task.
        __task_context_save_and_switch (
            &k->tasks[0].sp,      //Kernel context stack pointer saved here.
            k->tasks[k->task].sp  //Context set to task stack pointer. 
        );

//When an exception or interrupt occurs, context is switched from running
//task to here.
        uart_puts("rpi3rtos::kernel_main(): Servicing exception...\n");

        if (k->ticks > 0) {
            uart_puts("rpi3rtos::task0_start(): ");
            uart_u64hex_s(k->ticks);
            uart_puts(" ticks elapsed since last kernel service.\n");
            k->ticks = 0;
        }

        if (0x1 == k->syscall) { //task requesting suspend.
            uart_puts("rpi3rtos::kernel_main(): Task requesting suspend...\n");
            uart_puts("rpi3rtos::kernel_main(): Task sp = ");
            uart_u64hex_s(k->tasks[k->task].sp);
            uart_puts("\n");
        }

        uart_puts("rpi3rtos::kernel_main(): Exception serviced.\n");
   }
}
