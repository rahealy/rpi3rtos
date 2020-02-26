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

#include "irq.h"
#include "mmu.h"
#include "uart.h"
#include "timer.h"

//
//kernel_task_lst{}
// Routines which implement the linked list and btree functionality.
//

//
//kernel_task_lst_init()
// Initialize task list.
//
void kernel_task_lst_init(kernel_task_lst *lst) {
    u64_t i;

    lst->items[0].list = lst;
    lst->items[0].prev = 0;
    lst->items[0].next = &lst->items[1];

    for (i = 1; i < KERNEL_TASKS_MAX; ++i) {
        lst->items[i].list = lst;
        lst->items[i].next = &lst->items[i+1];
        lst->items[i].prev = &lst->items[i-1];
    }

    lst->stack = &lst->items[0];
    lst->root = 0;
}

//
//kernel_task_lst_psh_ll()
// Push a new node item onto the top of a linked list.
//
int kernel_task_lst_psh_ll(kernel_task_lst *lst, u64_t task) {
    kernel_nd_item *new = lst->stack;

    if (0 == new) { //This should never happen.
        uart_puts("rpi3rtos::kernel_task_lst_psh_ll(): Out of tasks in linked list. Panic.\n");
        kernel_panic();
    }

    lst->stack = new->next;
    lst->stack->prev = 0;

    new->task = task;
    new->next = lst->root;
    new->prev = 0;

    if (new->next) {
        new->next->prev = new;
    }

    lst->root = new;

    return 0;
}

//
//kernel_nd_item{}
// Routines which implement the linked list functionality.
//

//
//kernel_nd_item_rmv_ll()
// Remove node item from linked list. Item pointer no longer valid.
//
// Return next available item or null if list empty.
//
kernel_nd_item* kernel_nd_item_rmv_ll(kernel_nd_item *item) {
    kernel_nd_item *next = item->next ? item->next : item->prev;
    
    item->prev->next = item->next;
    item->next->prev = item->prev;

    item->next              = item->list->stack;
    item->prev              = item->list->stack->prev;
    item->list->stack->prev = item;
    item->list->stack       = item;
    
    return next;
}

//
//kernel{}
// Routines that implement kernel functionality.
//

//From vectors.S
extern void __exception_vectors_start(void);
extern u64_t __task_context_save(u64_t);

//From exceptions.c
extern void exceptions_init(kernel *k);

//From interrupts.c
extern void interrupts_init(kernel *k);

//
//kernel_panic()
// Infinite loop.
//
void kernel_panic() {
    while(1) {
        asm("wfe":::);
    }
}

//
//kernel_init()
// Initialize all things kernel.
//
int kernel_init(kernel *k, u64_t num_tasks) {
    u64_t i;
    u64_t base = task_get_base_addr(0);

    uart_puts("rpi3rtos::kernel_init(): Initializing kernel (");
    uart_u64hex_s((u64_t) k);
    uart_puts(") ");
    uart_u64hex_s(num_tasks);
    uart_puts(" tasks...\n");

//Make sure there aren't more tasks then we can handle.
    if (num_tasks > KERNEL_TASKS_MAX) {
        uart_puts("rpi3rtos::kernel_init(): Number of tasks exceeds maximum allowed. Panic.\n");
        kernel_panic();
    } else {
        k->num_tasks = num_tasks;
    }

//Tell the exceptions and interrupts where the kernel is stored.
    exceptions_init(k);
    interrupts_init(k);

//Set exception handlers for EL1.
    uart_puts("rpi3rtos::kernel_init(): Set exception handler vector to ");
    uart_u64hex_s((u64_t) __exception_vectors_start + base);
    uart_puts("\n");
    asm volatile ("msr  vbar_el1, %0\n" :: "r"(__exception_vectors_start + base) :);

//Initialize the list of sleeping tasks.
    kernel_task_lst_init(&k->sleeping);

//Initialize the priority queue.
    kernel_task_lst_init(&k->queue);

//Initialize list of tasks.
    uart_puts("rpi3rtos::kernel_init(): Initializing kernel task headers...\n");
    for (i = 0; i < k->num_tasks; ++i) {
        k->tasks[i].header = task_get_header(i); 
        k->tasks[i].priority = 0;
        k->tasks[i].sp = task_get_base_addr(i);
    }
    uart_puts("rpi3rtos::kernel_init(): Done.\n");

//Initialize the actual tasks themselves.
    uart_puts("rpi3rtos::kernel_init(): Initializing tasks ");
    uart_u64hex_s(1);
    uart_puts("-");
    uart_u64hex_s(k->num_tasks - 1);
    uart_puts("...\n");

    for (k->task = 1; k->task < k->num_tasks; ++k->task) {
        u64_t sp;

        uart_puts("rpi3rtos::kernel_init(): Initializing task ");
        uart_u64hex_s(k->task);
        uart_puts("\n");

        uart_puts("rpi3rtos::kernel_init(): Task SP is ");
        uart_u64hex_s(k->tasks[k->task].sp);
        uart_puts("\n");

        uart_puts("rpi3rtos::kernel_init(): Pointer to kernel task SP located at ");
        uart_u64hex_s((u64_t) &k->tasks[k->task].sp);
        uart_puts("\n");

        uart_puts("rpi3rtos::kernel_init(): Kernel task SP is ");
        uart_u64hex_s(k->tasks[0].sp);
        uart_puts("\n");

        asm("nop");
        asm("nop");
        asm("nop");
        uart_puts("rpi3rtos::kernel_init(): Kernel SP is ");
        asm("mov %0, sp" : "=r"(sp) : : );
        uart_u64hex_s(sp);
        uart_puts("\n");
        asm("nop");
        asm("nop");
        asm("nop");

//Switch to task context and call main.
        __task_context_save_and_branch (
            &k->tasks[0].sp,                        //Kernel context stack pointer saved here.
            k->tasks[k->task].sp,                   //Context set to task stack pointer. 
            (u64_t) k->tasks[k->task].header->main  //Function called in new context.
        );

//Execution resumes here after task->main() initializes and calls task_suspend().
        uart_puts("rpi3rtos::kernel_init(): Task suspended. \n");
        uart_puts("rpi3rtos::kernel_init(): Suspended task SP is ");
        uart_u64hex_s(k->tasks[k->task].sp);
        uart_puts("\n");
    }

    uart_puts("rpi3rtos::kernel_init(): Kernel tasks initialized.\n");

//Initialize kernel specifics.
    k->task    = 1; //First non-kernel task is always 0x1. 
    k->ticks   = 0; //Reset
    k->syscall = 0; //Reset
    k->sysarg  = 0; //Reset

    uart_puts("rpi3rtos::kernel_init(): Kernel initialized.\n");

    return 0;
}

//
//kernel_service_sleeping()
// If pending ticks then service the sleeping tasks. Remove ready to 
// wake tasks from the sleeping list and insert in the priority queue.
//
void kernel_service_sleeping(kernel *k) {
    if (k->ticks > 0) {
        kernel_nd_item *cur;

        uart_puts("rpi3rtos::kernel_service_sleeping(): Timer tick: ");
        uart_u64hex_s(k->ticks);
        uart_puts(" ticks elapsed.\n");

        for(cur = k->sleeping.root; cur > 0; cur = cur->next) {
            --k->tasks[cur->task].wakeup;
            if (0 == k->tasks[cur->task].wakeup) {
                uart_puts("rpi3rtos::kernel_service_sleeping(): Waking up task ");
                uart_u64hex_s(cur->task);
                uart_puts("\n");
                cur = kernel_nd_item_rmv_ll(cur);
            }
        }

        k->ticks = 0;
    }
}


//
//kernel_ins_priority_queue()
// Insert a new node item into the kernel's priority queue.
//
void kernel_ins_priority_queue(kernel *k, u64_t task) {
    kernel_nd_item *new = k->queue.stack;
    kernel_nd_item *cur = k->queue.root;

    if (0 == new) { //This should never happen.
        uart_puts("rpi3rtos::kernel_ins_priority_queue(): Out of tasks in linked list. Panic.\n");
        kernel_panic();
    }

    k->queue.stack = new->next;
    k->queue.stack->prev = 0;

    new->task = task;

    if (cur) {
//List sorted high to low priority in order of insertion.
        while(1) {
            if (k->tasks[cur->task].priority >= k->tasks[task].priority) && {
                if (cur->next) { //Keep searching.
                    cur = cur->next;
                } else { 
                    //Insert new at end of list.
                    new->next = 0;
                    new->prev = cur;
                    cur->next = new;
                    break;
                }
            } else { 
                //Insert new before cur.
                new->next = cur;
                new->prev = cur->prev;
                cur->prev = new;
                break;
            }
        }
    } else {
        //First task in queue.
        new->next = 0;
        new->prev = 0;
        k->queue.root = new;
    }
}

void kernel_task_flags_print_debug(u64_t flags) {
    uart_puts("rpi3rtos::kernel_task_flags_print_debug() Task flags: \n")

    if (flags & KERNEL_TASK_FLAG_WAKEUP_POST_INIT) {
        uart_puts("KERNEL_TASK_FLAG_WAKEUP_POST_INIT\n");
    }

    if (flags & KERNEL_TASK_FLAG_WAKEUP_UART0_RX) {
        uart_puts("KERNEL_TASK_FLAG_WAKEUP_UART0_RX\n");
    }
}

void kernel_service_syscall(kernel *k) {
    switch(k->syscall) {
        case 0:
        break;

        case KERNEL_SYSCALL_SLEEP:
            uart_puts("rpi3rtos::kernel_main(): Task requesting sleep...\n");

            uart_puts("rpi3rtos::kernel_main(): Requested sleep duration (ms): ");
            uart_u64hex_s(k->sysarg);
            uart_puts("\n");

            uart_puts("rpi3rtos::kernel_main(): Rounded up to nearest kernel tick (ticks): ");
            uart_u64hex_s((k->sysarg + KERNEL_TICK_DURATION_MS - 1) / KERNEL_TICK_DURATION_MS);
            uart_puts("\n");

            k->tasks[k->task].flags |= k->sysarg | KERNEL_TASK_FLAG_SLEEPING;
            k->syscall = 0;
            k->sysarg  = 0;
            
//Update current task.
        break;

        case KERNEL_SYSCALL_SUSPEND:
            uart_puts("rpi3rtos::kernel_service_syscall(): Task requesting suspend...\n");

            uart_puts("rpi3rtos::kernel_service_syscall(): Task sp = ");
            uart_u64hex_s(k->tasks[k->task].sp);
            uart_puts("\n");

            kernel_task_flags_print_debug(k->sysarg);

            k->tasks[k->task].flags |= k->sysarg | KERNEL_TASK_FLAG_SUSPENDED;
            k->syscall = 0;
            k->sysarg  = 0;
        break;

        default:
        break;
    };
        if (KERNEL_SYSCALL_SUSPEND == k->syscall) { //task requesting suspend.
//FIXME: Check task priority list. If no tasks to run then stay in kernel.
            k->task    = 0;
        }

        if (KERNEL_SYSCALL_SLEEP == k->syscall) { //Current task requesting sleep.

//             uart_puts("rpi3rtos::kernel_main(): Task sp = ");
//             uart_u64hex_s(k->tasks[k->task].sp);
//             uart_puts("\n");

//FIXME: Check task priority list. If no tasks to run then stay in kernel.
            k->task    = 0;
            k->syscall = 0;
            k->sysarg  = 0;
        }
}

void kernel_main(kernel *k) {
    u64_t base = task_get_base_addr(0);

    uart_puts("rpi3rtos::kernel_main(): Entering main kernel loop for kernel stored at ");
    uart_u64hex_s((u64_t) k);
    uart_puts(".\n");

    //Set exception handlers for EL1.
    uart_puts("rpi3rtos::kernel_main(): Rebased __exception_vectors_start: ");
    uart_u64hex_s((u64_t) __exception_vectors_start + base);
    uart_puts("\n");
    asm volatile ("msr  vbar_el1, %0\n" :: "r"(__exception_vectors_start + base) :);

    //Set slice timer.
    uart_puts("rpi3rtos::kernel_main(): Setting slice timer...\n");
    irq_disable();
    timer_init_core0(KERNEL_TICK_DURATION_MS);
    irq_enable();
    uart_puts("rpi3rtos::kernel_main(): Done setting slice timer.\n");

    while(1) {
//When an exception or interrupt occurs, context is switched from running
//task to here.
        irq_disable();

        kernel_service_sleeping(k);
        kernel_service_syscall(k);

        irq_enable();

        if (k->task) {
//Switch to currently running task.
            uart_puts("rpi3rtos::kernel_main(): Resume task ");
            uart_u64hex_s(k->task);
            uart_puts(".\n");

            __task_context_save_and_switch (
                &k->tasks[0].sp,      //Kernel context stack pointer saved here.
                k->tasks[k->task].sp  //Context set to task stack pointer. 
            );
        }
   }
}
