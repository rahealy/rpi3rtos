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

//
//KERNEL_TICK_DURATION_MS
// The duration of one tick (slice) of time in milliseconds.
//
#define KERNEL_TICK_DURATION_MS 1000 //FIXME: One second for debugging.

//
//KERNEL_TASKS_MAX
// Maximum number of tasks kernel can handle.
//
#define KERNEL_TASKS_MAX 8

//*********************************************************************
//
//KERNEL_TASK_FLAG_*
// Task flags are used by kernel to determine task status.
//
//*********************************************************************

//
//KERNEL_TASK_FLAG_WAKEUP_POST_INIT
// After task has initialized it should suspend and return control to
// kernel using this flag.
//
#define KERNEL_TASK_FLAG_WAKEUP_POST_INIT (0x1)

//
//KERNEL_TASK_FLAG_WAKEUP_UART0_RX
// If UART0 (PLO11) received data wake task up.
//
#define KERNEL_TASK_FLAG_WAKEUP_UART0_RX (0x2)

//FIXME: I2S? I2C? SPI? Timers? DMA? Should IRQs from these sources be
//FIXME: serviced by privileged driver tasks instead of the kernel?

//
//KERNEL_TASK_FLAG_SUSPENDED
// Task is suspended.
//
#define KERNEL_TASK_FLAG_SUSPENDED (0x3)

//
//KERNEL_TASK_FLAG_SLEEPING
// Task is sleeping.
//
#define KERNEL_TASK_FLAG_SLEEPING (0x4)

//*********************************************************************
//
//KERNEL_SYSCALL_*
// The kernel supports syscalls via the ARM 'svc' opcode:
// asm("svc %0" :: "r"(KERNEL_SYSCALL_SUSPEND): );
//
// Arguments are passed in registers (x0..xN). See below for details.
//
//*********************************************************************

//
//KERNEL_SYSCALL_SUSPEND
// Suspend task indefinitely.
//
// x0 Contains logical 'or' of KERNEL_TASK_FLAG_WAKEUP_* conditions.
//
#define KERNEL_SYSCALL_SUSPEND 0x1

//
//KERNEL_SYSCALL_SLEEP
// Suspend task for a period of time in milliseconds rounded up to 
// nearest kernel tick duration.
//
// x0 Contains the number of milliseconds.
//
#define KERNEL_SYSCALL_SLEEP   0x2

//
//kernel_nd_item{}
// Data structure representing items in a linked list or nodes in a
// binary tree.
//
typedef struct _kernel_nd_item {
    union {
        struct {
            struct _kernel_task_lst *list;
            struct _kernel_nd_item  *next;
            struct _kernel_nd_item  *prev;
        };

        struct  {
            struct _kernel_nd_item *parent;
            struct _kernel_nd_item *left;
            struct _kernel_nd_item *right;
        };
    };
    u64_t task;
} kernel_nd_item;


//
//_kernel_task_lst{}
// A task list can be a linked list or a btree.
//
//FIXME: Haven't decided if b-tree is necessary. Unimplemented for now.
//FIXME: Arrays might be good enough? Use ll's for now.
//
typedef struct _kernel_task_lst {
    kernel_nd_item *root;  //Root of tree / head of linked list.
    kernel_nd_item *stack; //Stack of unused items.
    kernel_nd_item items[KERNEL_TASKS_MAX]; //Storage.
} kernel_task_lst;


//
//kernel_task{}
// Task state information kept by the kernel.
//
typedef struct _kernel_task {
    task_header *header;  //Points at the task header.
    i64_t       priority; //Task priority used to determine which gets slices of time.
    u64_t       flags;    //Logical or of KERNEL_TASK_FLAG_*
    u64_t       sp;       //Task stack pointer used to save/restore context.
    u64_t       wakeup;   //Number of slices before sleeping task put back on priority queue.
} kernel_task;

//
//kernel{}
// Kernel structure maintains task states.
//
typedef struct _kernel {
    u64_t task;                 //Currently running task.
    u64_t ticks;                //Number of ticks since kernel last serviced.
    u64_t syscall;              //Non-zero if syscall needs to be serviced.
    u64_t sysarg;               //Argument to syscall.
    u64_t num_tasks;            //Number of tasks. Set by loader.
    kernel_task_lst sleeping;   //Sleeping tasks.
    kernel_task_lst queue;      //Priority queue.
    kernel_task tasks[KERNEL_TASKS_MAX]; //Tasks.
} kernel;

//
//__task_context_save_and_branch()
// Save current context and store stack pointer in sp_saved. Switch to
// stack pointer for task context in sp_new and call function at address.
//
extern volatile void __task_context_save_and_branch(volatile u64_t *sp_saved, 
                                                    volatile u64_t sp_new, 
                                                    volatile u64_t address);

//
//__task_context_save_and_switch()
// Save current context and store stack pointer in sp_saved. Switch to
// stack pointer for task context in sp_new. Restore context.
//
extern volatile void __task_context_save_and_switch(volatile u64_t *sp_saved, 
                                                    volatile u64_t sp_new);

//
//kernel_init()
// Initialize the provided kernel structure.
//
int kernel_init(kernel *k, u64_t num_tasks);

//
//kernel_main()
// Main kernel loop. Sets slice timer, processes exceptions and 
// interrupts, and performs task management.
//
void kernel_main(kernel *k);

//
//kernel_panic()
// Goes into an infinite loop.
//
void kernel_panic(void);

//
//kernel_get_sp
// Get the kernel stack pointer offset.
//
inline u64_t kernel_get_sp(kernel *k) {
    return k->tasks[0].sp;
}

//
//kernel_get_cur_task_sp_ptr
// Get a pointer to a kernel task structure's SP field for the 
// currently running task.
//
inline u64_t *kernel_get_cur_task_sp_ptr(kernel *k) {
    return &k->tasks[k->task].sp;
}

#endif
