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
//KERNEL_TASK_FLAG_SUSPENDED
// Task is suspended.
//
#define KERNEL_TASK_FLAG_SUSPENDED (0x1)


//
//KERNEL_TASK_FLAG_SLEEPING
// Task is sleeping.
//
#define KERNEL_TASK_FLAG_SLEEPING (0x1 << 1)

//*********************************************************************
//
//KERNEL_TASK_FLAG_WAKEUP_*
// Task flags used by kernel to determine when to wake up a suspended
// task.
//
//*********************************************************************

//
//KERNEL_TASK_FLAG_WAKEUP_POST_INIT
// After task has initialized it should suspend and return control to
// kernel using this flag.
//
#define KERNEL_TASK_FLAG_WAKEUP_POST_INIT (0x1 << 2)

//
//KERNEL_TASK_FLAG_WAKEUP_POST_RESET
// After task has been reset it should suspend and return control to
// kernel using this flag. Kernel will reset the task's stack pointer
// and re-initialize the task.
//
#define KERNEL_TASK_FLAG_WAKEUP_POST_RESET (0x1 << 3)

//
//KERNEL_TASK_FLAG_WAKEUP_CLEAR_ALL
// Logical and to clear all wakeup flags.
//
#define KERNEL_TASK_FLAG_WAKEUP_CLEAR_ALL (~(KERNEL_TASK_FLAG_WAKEUP_POST_INIT & \
                                             KERNEL_TASK_FLAG_WAKEUP_POST_RESET))

//*********************************************************************
//
//KERNEL_TASK_FLAG_QUEUE_*
// Task flags used by kernel to determine how to manage queue priority
// for a task.
//
//*********************************************************************

//
//KERNEL_TASK_FLAG_QUEUE_ROUND_ROBIN
// After tick kernel will move task to the end of the list of tasks 
// with the same priority and move to next task with same priority in
// the queue.
//
#define KERNEL_TASK_FLAG_QUEUE_ROUND_ROBIN (0x1 << 4)

//
//KERNEL_TASK_FLAG_QUEUE_FIFO
// After tick kernel will finish task before moving to the next task
// with the same priority in the queue.
//
#define KERNEL_TASK_FLAG_QUEUE_FIFO (0x1 << 5)

//
//KERNEL_TASK_FLAG_QUEUE_CLEAR_ALL
// Logical and to clear all queue flags.
//
#define KERNEL_TASK_FLAG_QUEUE_CLEAR_ALL (~(KERNEL_TASK_FLAG_QUEUE_ROUND_ROBIN & \
                                            KERNEL_TASK_FLAG_QUEUE_FIFO))


//
//FIXME: UART0, UART1, I2S? I2C? SPI? Timers? DMA? Should IRQs from 
//FIXME: these sources be serviced by privileged driver tasks instead 
//FIXME: of the kernel?
//
// If UART0 (PLO11) received data wake task up.
//
//#define KERNEL_TASK_FLAG_WAKEUP_UART0_RX (0x2)
//


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
//KERNEL_SYSCALL_PRIORITY
// Change task priority in queue. If priority is lower then current
// task will be suspended and moved down in the queue.
//
// x0 bits [31..0] Contain priority 1-N (lowest to highest).
//    bits [63..32] Contain one of the KERNEL_TASK_FLAG_QUEUE_* flags.
//
#define KERNEL_SYSCALL_PRIORITY   0x3

//
//kernel_nd_item{}
// Data structure representing items in a linked list or nodes in a
// binary tree.
//
typedef struct _kernel_nd_item {
    union {
        struct {
            struct _kernel_nd_lst   *list;
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
//kernel_nd_lst{}
// Linked list. Implements a FIFO.
//
typedef struct _kernel_nd_lst {
    kernel_nd_item *head;
    kernel_nd_item *tail;
} kernel_nd_lst;

//
//kernel_sysarg{}
// Arguments to syscalls may only need to be 32bits.
//
typedef struct _kernel_sysarg {
    union {
        struct {
            u32_t lo;
            u32_t hi;
        };
        u64_t value;
    };
} kernel_sysarg;

//
//kernel_task{}
// Task state information kept by the kernel.
//
typedef struct _kernel_task {
    task_header *header;  //Points at the task header.
    i64_t priority;       //Task priority used to determine which gets slices of time.
    u64_t flags;          //Logical or of KERNEL_TASK_FLAG_*
    u64_t sp;             //Task stack pointer used to save/restore context.
    i64_t wakeup;         //Number of slices before sleeping task put back on priority queue.
    kernel_nd_item node;  //Node in priority queue.
} kernel_task;

//
//kernel{}
// Kernel structure maintains task states.
//
typedef struct _kernel {
    u64_t task;                 //Currently running task.
    u64_t ticks;                //Number of ticks since kernel last serviced.
    u64_t syscall;              //Non-zero if syscall needs to be serviced.
    kernel_sysarg sysarg;       //Argument to syscall.
    u64_t num_tasks;            //Number of tasks. Set by loader.
    kernel_nd_item *queue;      //Priority queue.
    kernel_nd_lst  sleep;      //Sleeping tasks. FIFO.
    kernel_nd_lst  suspend;    //Suspended tasks. FIFO.
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
//Returns the address of the kernel in kernel space.
//
kernel *kernel_get_pointer();

//
//kernel_get_sp
// Get the kernel stack pointer offset.
//
inline u64_t kernel_get_sp() {
    return kernel_get_pointer()->tasks[0].sp;
}

//
//kernel_get_cur_task_sp_ptr
// Get a pointer to a kernel task structure's SP field for the 
// currently running task.
//
u64_t *kernel_get_cur_task_sp_ptr();

#endif
