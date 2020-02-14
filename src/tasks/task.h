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

#ifndef TASK_H
#define TASK_H

//
//task.h
// A task is a block of code that will be run.
//
#include "platform.h"

#define TASK_TIMED_OUT 0x00000001 //Set if task is still running after slice expires.
#define TASK_WORKING   0x00000002 //Task is working on something.
#define TASK_PENDING   0x00000004 //Task is waiting on something.
#define TASK_FINISHED  0x00000008 //Task is finished.

typedef void (*taskfn)();

typedef struct _task_header {
    u64_t flags;    //Status flags.
    u64_t sp;       //Saved stack pointer.
    taskfn reset;   //Reset/re-init the task. Reset should check flags to see why.
    taskfn main;    //Task's main loop.
} task_header;

//
//task_save_context()
// Saves the task's state on the current stack.
//
void __attribute__((naked)) task_save_context();

//
//task_restore_context()
// Restores the task's state from the current stack.
//
void __attribute__((naked)) task_restore_context();

#endif