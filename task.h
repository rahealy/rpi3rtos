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
//task.h
// A task is a block of code that will be run.
//
#include "platform.h"

#define TASK_TIMED_OUT 0x00000001
#define TASK_WORKING   0x00000002
#define TASK_IDLE      0x00000004

typedef void (*taskfn)();

typedef struct _task_header {
    u64_t flags;
    u64_t sp;
    taskfn reset;
    taskfn main;
} task_header;

//
//task_save_state()
// Saves the task's state on the current stack.
//
void __attribute__((naked)) task_save_state();

//
//task_restore_state()
// Restores the task's state from the current stack.
//
void __attribute__((naked)) task_restore_state();
