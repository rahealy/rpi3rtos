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
// A task is a position independent executable that is managed by a 
// kernel.
//

#include "platform.h"
#include "mmu.h"


//
//taskfn()
// Signature of the two task functions called by kernel.
//
typedef void (*taskfn)(u64_t);

//
//TASK_*_MAGIC
// Magic numbers are used as a sanity check by startup when loading
// tasks into the address space from the kernel image.
//
#define TASK_LIST_ITEM_MAGIC 0x4D455449 //"ITEM"
#define TASK_HEADER_MAGIC    0x4B534154 //"TASK"

//
//task_list_item{}
// List item for each task stored in the executable image. This gets
// loaded into first block of r/o memory in task's address space.
//
typedef struct _task_list_item {
    u64_t magic;   //Always ASCII 'ITEM'
    u64_t ro_end;  //End of R/O code + data segment starting at 0x0;
    u64_t rw_beg;  //4kB aligned beginning of R/W memory.
    u64_t rw_end;  //End of R/W memory. BSS follows.
    u64_t bss_beg; //BSS.
    u64_t bss_end; //BSS.
} task_list_item; 


//
//TASK_HEADER_FLAG_OVERSLEPT
// If task was sleeping and kernel couldn't wake it up in time kernel
// sets this flag.
//
#define TASK_HEADER_FLAG_OVERSLEPT 0x1

//
//task_header{}
// Header shared by kernel and task. This gets loaded into first block
// of 4kB aligned r/w memory.
//
typedef struct _task_header {
    u64_t magic;         //Always ASCII 'TASK'
    u64_t flags;         //Status flags.
    i64_t priority;      //Priority. Use task_priority_set() to change.
    i64_t priority_flgs; //Priority flags. Use task_priority_set() to change.
    taskfn init;         //Initialize task then suspend.
    taskfn reset;        //Reset the task then suspend.
} task_header;

//FIXME: Need macros to build & init task_list_item & task_header correctly.

//
//task_get_base_addr()
// Tasks are stored at the beginning of the highest block by memory address
// of task address space . Stack grows toward lower memory addresses to fill
// lower blocks.
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
//task_suspend()
// Suspend current task and return control to kernel.
// 
// wakeup - Logical or of 0 or more KERNEL_TASK_FLAG_WAKEUP_* flags.
//
void task_suspend(u64_t wakeup);

//
//task_sleep()
// Suspend task for a period of time in milliseconds rounded up to 
// nearest kernel tick duration.
//
void task_sleep(u64_t msecs);

//
//task_priority_set()
// Set the task's priority. Task may be suspended if priority change
// moves it lower in the priority queue. Flags can be 0 (no change)
// or one of the KERNEL_TASK_FLAG_QUEUE_* values.
//
void task_priority_set(u64_t priority, u64_t flags);

#endif
