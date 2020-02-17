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
#include "mmu.h"

#define TASK_TIMED_OUT 0x00000001 //Set if task is still running after slice expires.
#define TASK_WORKING   0x00000002 //Task is working on something.
#define TASK_PENDING   0x00000004 //Task is waiting on something.
#define TASK_FINISHED  0x00000008 //Task is finished.

typedef void (*taskfn)();

#define TASK_LIST_ITEM_MAGIC 0x4D455449 //"ITEM"

//
//task_list_item
// List header for each task stored in the executable image. This is
// not loaded into the task's address space.
//
// magic - always ASCII 'ITEM'
// next  - byte offset of the next task in the image.
// sz    - size in bytes of current task.
//
typedef struct _task_list_item {
    u64_t magic;
    u64_t sz;
} task_list_item; 

//
//Header shared by kernel and task.
//
typedef struct _task_header {
    u64_t flags;    //Status flags.
    u64_t sp;       //Saved stack pointer.
    taskfn start;   //Run once. Zero bss and stuff.
    taskfn reset;   //Reset/re-init the task. Check flags for timeout.
    taskfn main;    //Task's main loop.
} task_header;

//FIXME: Need macros to build & init task_list_item & task_header correctly.

//
//task_get_base_addr()
// Tasks are stored at the bottom most 2MB boundary.
//
inline u64_t task_get_base_addr(u64_t task) {
    return ((task + 1) * MMU_TASK_MEMORY_SZ) - MMU_BLOCK_SZ;
}

//
//task_get_header()
// Get a pointer to the task's header.
//
inline task_header* task_get_header(u64_t task) {
    return (task_header *) (task_get_base_addr(task) + sizeof(task_list_item));
}

//
//task_header_rebase()
// After the task has been loaded into memory rebase the header to
// actual locations rather than relative.
//
void task_header_rebase(u64_t task);


//
//task_zero_bss()
// Task start() must zero the task's bss segment.
//
void task_zero_bss(char *bss, u64_t sz);

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
