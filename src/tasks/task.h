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

typedef void (*taskfn)(u64_t);

#define TASK_LIST_ITEM_MAGIC 0x4D455449 //"ITEM"
#define TASK_HEADER_MAGIC 0x4B534154    //"TASK"

//
//task_list_item
// List header for each task stored in the executable image. This is
// not loaded into the task's address space.
//
typedef struct _task_list_item {
    u64_t magic;   //Always ASCII 'ITEM'
    u64_t ro_end;  //End of R/O code + data segment starting at 0x0;
    u64_t rw_beg;  //4kB aligned beginning of R/W memory.
    u64_t rw_end;  //End of R/W memory. BSS follows.
    u64_t bss_beg; //BSS.
    u64_t bss_end; //BSS.
} task_list_item; 

#define TASK_HEADER_FLAG_RUNNING 0x1
#define TASK_HEADER_FLAG_TIMEOUT (0x1 << 1)

//
//Header shared by kernel and task.
//
typedef struct _task_header {
    u64_t magic;        //Magic value.
    u64_t flags;        //Status flags.
    taskfn init;        //Run once. Constructor.
    taskfn reset;       //Reset/re-init the task. Check flags for timeout.
    taskfn start;       //Task's main loop.
} task_header;

//FIXME: Need macros to build & init task_list_item & task_header correctly.

//
//task_get_base_addr()
// Tasks are stored at the bottom most 2MB boundary. Stack grows toward
// lower address.
//
inline u64_t task_get_base_addr(u64_t task) {
    return ((task + 1) * MMU_TASK_MEMORY_SZ) - MMU_BLOCK_SZ;
}

//
//task_get_list_item()
// Tasks are stored at the bottom most 2MB boundary.
//
task_list_item *task_get_list_item(u64_t task);

//
//task_get_header()
// Get a pointer to the task's header.
//
task_header *task_get_header(u64_t task);


//
//task_header_rebase()
// After the task has been loaded into memory rebase the header to
// actual locations rather than relative.
//
void task_header_rebase(u64_t task);


//
//task_bss_zero()
// Zero the task's bss segment.
//
void task_bss_zero(u64_t task);

//
//Suspend current task and return control to kernel.
//
void task_suspend(void);

//
//Suspend current task for specified number of milliseconds.
//
void task_sleep(u64_t msecs);

#endif
