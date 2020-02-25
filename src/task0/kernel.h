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

#ifndef KERNEL_H
#define KERNEL_H

#include "platform.h"
#include "task.h"

#define KERNEL_TASKS 8

typedef struct _kernel_task {
    task_header *header;
    u64_t       priority;
    u64_t       sp;
} kernel_task;

//
//Task asks for block of time from kernel. Gets it.
//After block of time task is stopped and kernel dispatches messages.
//Up to task to decide if too much time has elapsed.
//
typedef struct _kernel {
    u64_t task;                 //Currently running task.
    u64_t ticks;                //Number of ticks since kernel last serviced.
    u64_t syscall;              //Non-zero if syscall needs to be serviced.
    u64_t num_tasks;            //Number of tasks. Set by loader.
    kernel_task tasks[KERNEL_TASKS]; //Tasks.
} kernel;


//
// __task_context_save_and_branch()
//  Save current context and store stack pointer in sp_saved. Switch to
//  stack pointer for task context in sp_new and call function at address.
//
extern volatile void __task_context_save_and_branch(volatile u64_t *sp_saved, 
                                                    volatile u64_t sp_new, 
                                                    volatile u64_t address);

//
// __task_context_save_and_switch()
//  Save current context and store stack pointer in sp_saved. Switch to
//  stack pointer for task context in sp_new. Restore context.
//
extern volatile void __task_context_save_and_switch(volatile u64_t *sp_saved, 
                                                    volatile u64_t sp_new);

int kernel_init(volatile kernel *k, u64_t num_tasks);
void kernel_tasks_init(volatile kernel *k);
void kernel_main(volatile kernel *k);

#endif
