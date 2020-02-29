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
//kernel{}
// Routines that implement kernel functionality.
//

//
//From linker script.
//
extern u64_t __kernel_pointer;

//
//Global kernel pointer.
//
kernel *g_kernel_pointer
    __attribute__ ((section (".kernel_pointer"))) 
    __attribute__ ((__used__)) = 0;

kernel *kernel_get_pointer() {
    return g_kernel_pointer;
}

u64_t *kernel_get_cur_task_sp_ptr() {
    kernel *k = kernel_get_pointer();

    uart_puts("rpi3rtos::kernel_get_cur_task_sp_ptr(): "
              "current task is ");
    uart_u64hex_s(k->task);
    uart_puts("\n");

    uart_puts("rpi3rtos::kernel_get_cur_task_sp_ptr(): "
              "Pointer to kernel task SP located at ");
    uart_u64hex_s((u64_t) &k->tasks[k->task].sp);
    uart_puts("\n");

    return &k->tasks[k->task].sp;
}

//From vectors.S
extern void __exception_vectors_start(void);
extern u64_t __task_context_save(u64_t);

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
//kernel_task_flags_print()
// Helper function prints kernel task flags.
//
void kernel_task_flags_print(u64_t flags) {
    uart_puts("rpi3rtos::kernel_task_flags_print(): Task flags: \n");

    if (flags & KERNEL_TASK_FLAG_SUSPENDED) {
        uart_puts("rpi3rtos::kernel_task_flags_print(): KERNEL_TASK_FLAG_SUSPENDED\n");
    }

    if (flags & KERNEL_TASK_FLAG_SLEEPING) {
        uart_puts("rpi3rtos::kernel_task_flags_print(): KERNEL_TASK_FLAG_SLEEPING\n");
    }

    if (flags & KERNEL_TASK_FLAG_WAKEUP_POST_INIT) {
        uart_puts("rpi3rtos::kernel_task_flags_print(): KERNEL_TASK_FLAG_WAKEUP_POST_INIT\n");
    }

    if (flags & KERNEL_TASK_FLAG_WAKEUP_POST_RESET) {
        uart_puts("rpi3rtos::kernel_task_flags_print(): KERNEL_TASK_FLAG_WAKEUP_POST_RESET\n");
    }
}

//*********************************************************************
// Kernel Task Node Routines
//  Tasks are sorted and managed using referent nodes in data 
//  structures. Currently all structures are doubly linked lists. Other 
//  possibilities might include binary heaps or btrees.
//*********************************************************************

//
//kernel_queue_task_node_priority_gte()
// Returns non-zero if task a is greater than or equal in priority to
// task b.
//
inline u64_t kernel_queue_task_node_priority_gte(kernel *k, i64_t a, i64_t b) {
    return (k->tasks[a].priority >= k->tasks[b].priority);
}

//
//kernel_queue_task_node_priority_lt()
// Returns non-zero if task a priority is less than task b.
//
inline u64_t kernel_queue_task_node_priority_lt(kernel *k, i64_t a, i64_t b) {
    return (k->tasks[a].priority < k->tasks[b].priority);
}

//
//kernel_task_node_psh()
// Push a task node onto the provided linked list.
//
void kernel_task_node_psh(kernel *k, kernel_nd_item **head, u64_t task) {
    kernel_nd_item *nd = &k->tasks[task].node;
    if(*head) {
        nd->next = *head;
        (*head)->prev = nd;
    }
    *head = nd;
}

//
//kernel_task_node_rmv()
// Remove a task node from the provided linked list.
//
void kernel_task_node_rmv(kernel *k, kernel_nd_item **head, u64_t task) {
    kernel_nd_item *nd = &k->tasks[task].node;

    if(nd->prev) {
        nd->prev->next = nd->next;
        nd->prev = 0;
    } else {
        *head = nd->next;
    }

    if (nd->next) {
        nd->next->prev = nd->prev;
        nd->next = 0;
    }
}

//*********************************************************************
// Kernel Task Node List Routines
//  Tasks are sorted and managed using referent nodes in data 
//  structures. Currently all structures are doubly linked lists. Other 
//  possibilities might include binary heaps or btrees.
//*********************************************************************

void kernel_task_node_list_tail_psh(kernel *k,
                                    kernel_nd_lst *list, 
                                    u64_t task) 
{
    kernel_nd_item *nd = &k->tasks[task].node;
    if (list->tail) {
        list->tail->next = nd;
        nd->prev = list->tail;
        nd->next = 0;
        list->tail = nd;
    } else {
        nd->prev = 0;
        nd->next = 0;
        list->tail = nd;
        list->head = nd;
    }
}

void kernel_task_node_list_rmv(kernel *k, 
                               kernel_nd_lst *list, 
                               u64_t task) 
{
    kernel_nd_item *nd = &k->tasks[task].node;

    if(nd->prev) {
        nd->prev->next = nd->next;
    } else {
        list->head = nd->next;
    }

    if (nd->next) {
        nd->next->prev = nd->prev;
    } else {
        list->tail = nd->prev;
    }

    nd->next = 0;
    nd->prev = 0;
}

void kernel_task_node_list_validate(kernel_nd_lst *list) {
    kernel_nd_item *cur = list->head;
    kernel_nd_item *prev = 0;
    u64_t i = 0;

    if(list->head) {
        if (list->tail) {
            if (list->tail->next) {
                uart_puts("rpi3rtos::kernel_task_node_list_validate(): "
                        "tail->next set. Panic.\n");
                kernel_panic();
            }
        } else {
            uart_puts("rpi3rtos::kernel_task_node_list_validate(): "
                      "Head set but not tail. Panic.\n");
            kernel_panic();
        }
        if (list->head->prev) {
            uart_puts("rpi3rtos::kernel_task_node_list_validate(): "
                    "head->prev set. Panic.\n");
            kernel_panic();
        }
    } else if(list->tail) {
        uart_puts("rpi3rtos::kernel_task_node_list_validate(): "
                  "Tail set but not head. Panic.\n");
        kernel_panic();
    }

    while(cur) {
        uart_puts("rpi3rtos::kernel_task_node_list_validate(): Task ");
        uart_u64hex_s(cur->task);
        uart_puts("\n");

        ++i;
        prev = cur;
        cur  = cur->next;
        if(cur) {
            if (cur->prev != prev) {
                uart_puts("rpi3rtos::kernel_task_node_list_validate(): "
                "cur->prev does not match prev. Panic.\n");
                kernel_panic();
            }
        }
    }

    uart_puts("rpi3rtos::kernel_task_node_list_validate(): List length is ");
    uart_u64hex_s(i);
    uart_puts("\n");
}


//*********************************************************************
// Kernel Queue Specific Task Node Routines
//  Implements the priority queue operations for a given data 
//  structure. Currently a doubly linked list.
//*********************************************************************

//
//kernel_queue_task_node_move_front()
// Helper function for kernel_queue_task_update(). Moves a node toward
// the front of the queue if its priority has raised.
//
void kernel_queue_task_node_move_front(kernel *k,
                                       kernel_nd_item *cur, 
                                       kernel_nd_item *nd) 
{
//Find first node with lower priority and insert before.
    while(1) {
        if (cur->prev) {
            //Keep searching.
            cur = cur->prev;
        } else {
//Remove node from queue and insert at front of queue.
            kernel_task_node_rmv(k, &k->queue, nd->task);
            nd->next  = cur;
            nd->prev  = 0;
            cur->prev = nd;
            k->queue  = nd;
            break;
        }

        if (kernel_queue_task_node_priority_gte(k, cur->task, nd->task)) {
//Remove node from queue and insert after cur.
            kernel_task_node_rmv(k, &k->queue, nd->task);
            nd->next  = cur->next;
            nd->prev  = cur;
            cur->next = nd;

            if (nd->next) {
                nd->next->prev = nd;
            }

            break;
        }
    }
}

//
//kernel_queue_task_node_move_back()
// Helper function for kernel_queue_task_update(). Moves a node toward
// the back of the queue if its priority has lowered.
//
void kernel_queue_task_node_move_back(kernel *k,
                                    kernel_nd_item *cur, 
                                    kernel_nd_item *nd) 
{
//Find first node with higher priority and insert after.
    while(1) {
        if (cur->next) { //Keep searching.
            cur = cur->next;
        } else {
//Remove node from queue and insert at end of queue.
            kernel_task_node_rmv(k, &k->queue, nd->task);
            nd->next = 0;
            nd->prev = cur;
            cur->next = nd;
            break;
        }

        if (kernel_queue_task_node_priority_lt(k, cur->task, nd->task)) {
//Remove node from queue and insert before cur.
            kernel_task_node_rmv(k, &k->queue, nd->task);
            nd->next = cur;
            nd->prev = cur->prev;
            cur->prev = nd;

            if (nd->prev) {
                nd->prev->next = nd;
            } else {
                k->queue = nd;
            }
            break;
        }
    }
}

//
//kernel_queue_task_node_pos_update()
// A change in a task priority requires updating its position in the queue.
//
void kernel_queue_task_node_pos_update(kernel *k, u64_t task) {
    kernel_nd_item *nd = &k->tasks[task].node;

//Look toward the front of the queue.
    if(nd->prev) {
        if (kernel_queue_task_node_priority_lt(k, nd->prev->task, task)) {
            //Node task priority is higher and needs to be moved up.
            kernel_queue_task_node_move_front(k, nd->prev, nd);
            return;
        }
    }

//Look toward the back of the queue.
    if (nd->next) {
        if (kernel_queue_task_node_priority_gte(k, nd->next->task, task)) {
            //Node task priority is lower and needs to be moved down.
            kernel_queue_task_node_move_back(k, nd->next, nd);
            return;
        }
    }

//Node is still where it belongs in the queue. Nothing to do.
    return;
}


//*********************************************************************
// Kernel Specific Task Node Routines
//  Tasks are sorted and managed using referent nodes in data 
//  structures. These routines provide an API of sorts to decouple the
//  data structure from the kernel task node operation.
//*********************************************************************

//
//kernel_[queue,suspend,sleep]_node_[add, rmv]()
// These are helper functions to provide an api of sorts to facilitate
// experimentation with different data structures and algorithms.
//

//kernel_*_task_node_add
inline void kernel_queue_task_node_add(kernel *k, u64_t task) {
    kernel_task_node_psh(k, &k->queue, task);
    kernel_queue_task_node_pos_update(k, task);
    k->task = k->queue ? k->queue->task : 0; //Update current task.
}

inline void kernel_suspend_task_node_add(kernel *k, u64_t task) {
    k->tasks[task].flags |= KERNEL_TASK_FLAG_SUSPENDED;
    kernel_task_node_list_tail_psh(k, &k->suspend, task);
}

inline void kernel_sleep_task_node_add(kernel *k, u64_t task) {
    k->tasks[task].flags |= KERNEL_TASK_FLAG_SLEEPING;
    kernel_task_node_list_tail_psh(k, &k->sleep, task);
}

//kernel_*_task_node_rmv
inline void kernel_queue_task_node_rmv(kernel *k, u64_t task) {
    kernel_task_node_rmv(k, &k->queue, task);
    k->task = k->queue ? k->queue->task : 0; //Update current task.
}

inline void kernel_suspend_task_node_rmv(kernel *k, u64_t task) {
    k->tasks[task].flags &= ~KERNEL_TASK_FLAG_SUSPENDED;
    kernel_task_node_list_rmv(k, &k->suspend, task);
}

inline void kernel_sleep_task_node_rmv(kernel *k, u64_t task) {
    k->tasks[task].flags &= ~KERNEL_TASK_FLAG_SLEEPING;
    kernel_task_node_list_rmv(k, &k->sleep, task);
}

//*********************************************************************
// Kernel Queue Task Routines
//  Helper functions remove code duplication.
//*********************************************************************

void kernel_queue_task_suspend_and_update(kernel *k, u64_t task) {
    uart_puts("rpi3rtos::kernel_queue_task_suspend_and_update(): Suspending task ");
    uart_u64hex_s(task);
    uart_puts(". Flags: \n");
    kernel_task_flags_print(k->sysarg.value);

//Update task flags.
    k->tasks[task].flags |= k->sysarg.value;

//Update queue.
    uart_puts("rpi3rtos::kernel_queue_task_suspend_and_update(): Remove from queue.\n");
    kernel_queue_task_node_rmv(k, task);     //Remove from queue.
    kernel_suspend_task_node_add(k, task);   //Add to suspend list.
    kernel_task_node_list_validate(&k->suspend);
}

void kernel_queue_task_sleep_and_update(kernel *k, u64_t task) {
    u64_t wakeup = (k->sysarg.value + 
                    KERNEL_TICK_DURATION_MS - 1) /
                    KERNEL_TICK_DURATION_MS;

    uart_puts("rpi3rtos::kernel_queue_task_sleep_and_update(): Putting task ");
    uart_u64hex_s(task);
    uart_puts(" to sleep for ");
    uart_u64hex_s(k->sysarg.value);
    uart_puts(" ms rounded up to ");
    uart_u64hex_s(wakeup);
    uart_puts(" ticks.\n");

//Set wakeup time in kernel ticks.
    k->tasks[task].wakeup = wakeup;

//Update queue.
    uart_puts("rpi3rtos::kernel_queue_task_sleep_and_update(): Remove from queue.\n");
    kernel_queue_task_node_rmv(k, task);     //Remove from queue.
    kernel_sleep_task_node_add(k, task);     //Add to sleep list.

    kernel_task_node_list_validate(&k->sleep);
}


//*********************************************************************
// Kernel Routines
//  Implements a task switching kernel providing task suspend, sleep
//  and queue priority.
//*********************************************************************

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

    uart_puts("rpi3rtos::kernel_init(): g_kernel_pointer located at ");
    uart_u64hex_s((u64_t) &g_kernel_pointer);
    uart_puts("\n");

    g_kernel_pointer = k;

    uart_puts("rpi3rtos::kernel_init(): g_kernel_pointer set to ");
    uart_u64hex_s((u64_t) g_kernel_pointer);
    uart_puts("\n");

//Make sure there aren't more tasks then we can handle.
    if (num_tasks > KERNEL_TASKS_MAX) {
        uart_puts("rpi3rtos::kernel_init(): Number of tasks exceeds maximum allowed. Panic.\n");
        kernel_panic();
    } else {
        k->num_tasks = num_tasks;
    }

//Set exception handlers for EL1.
    uart_puts("rpi3rtos::kernel_init(): Set exception handler vector to ");
    uart_u64hex_s((u64_t) __exception_vectors_start + base);
    uart_puts("\n");
    asm volatile ("msr  vbar_el1, %0\n" :: "r"(__exception_vectors_start + base) :);

//Initialize kernel specifics.
    k->task    = 0; //Reset
    k->ticks   = 0; //Reset
    k->syscall = 0; //Reset
    k->queue   = 0; //Reset
    k->sleep.head   = 0; //Reset
    k->sleep.tail   = 0; //Reset
    k->suspend.head = 0; //Reset
    k->suspend.tail = 0; //Reset
    k->sysarg.value = 0; //Reset

//Task0 (kernel) is never in the priority queue.
    k->tasks[0].header    = task_get_header(0);
    k->tasks[0].priority  = 0;
    k->tasks[0].flags     = 0;
    k->tasks[0].sp        = task_get_base_addr(0);
    k->tasks[0].node.list = 0;
    k->tasks[0].node.next = 0;
    k->tasks[0].node.prev = 0;
    k->tasks[0].node.task = 0;

//Initialize and queue non-kernel tasks.
    uart_puts("rpi3rtos::kernel_init(): Initializing kernel task headers...\n");
    for (i = 1; i < k->num_tasks; ++i) {
        task_header *tskhdr   = task_get_header(i);
        k->tasks[i].header    = tskhdr;
        k->tasks[i].priority  = tskhdr->priority;
        k->tasks[i].flags     = tskhdr->priority_flgs;
        k->tasks[i].sp        = task_get_base_addr(i);
        k->tasks[i].node.list = 0;
        k->tasks[i].node.next = 0;
        k->tasks[i].node.prev = 0;
        k->tasks[i].node.task = i;
        kernel_queue_task_node_add(k, i);
        uart_puts("rpi3rtos::kernel_init(): k->task = ");
        uart_u64hex_s(k->task);
        uart_puts("\n");        
    }
    uart_puts("rpi3rtos::kernel_init(): Kernel task headers initialized.\n");

//Initialize the actual tasks themselves.
    uart_puts("rpi3rtos::kernel_init(): Initializing tasks ");
    uart_u64hex_s(1);
    uart_puts("-");
    uart_u64hex_s(k->num_tasks - 1);
    uart_puts("...\n");

    while (k->task) {
//Switch to task context and call init.
        __task_context_save_and_branch (
            &k->tasks[0].sp,                        //Kernel context stack pointer saved here.
            k->tasks[k->task].sp,                   //Context set to task stack pointer. 
            (u64_t) k->tasks[k->task].header->init  //Function called in new context.
        );

//Execution resumes here after task->init() initializes and calls task_suspend().
        if(KERNEL_SYSCALL_SUSPEND == k->syscall) {
            kernel_queue_task_suspend_and_update(k, k->task);
        } else {
            uart_puts("rpi3rtos::kernel_init(): Task made unexpected syscall ");
            uart_u64hex_s(k->syscall);
            uart_puts(". Panic. \n");
            kernel_panic();
        }

        k->syscall = 0; //Reset
        k->sysarg.value = 0; //Reset
    }

    uart_puts("rpi3rtos::kernel_init(): Kernel tasks initialized.\n");
    uart_puts("rpi3rtos::kernel_init(): Kernel initialized.\n");

    return 0;
}

//
//kernel_service_sleeping()
// If pending ticks then service the sleeping tasks. Remove ready to 
// wake tasks from the sleeping list and insert in the priority queue.
//
void kernel_service_sleeping(kernel *k) {
    kernel_nd_item *cur = k->sleep.head;

    while(cur) {
        if (k->tasks[cur->task].flags & KERNEL_SYSCALL_SLEEP) {
//Decrement task wakeup counter and check for wake up.
            k->tasks[cur->task].wakeup -= k->ticks;

            if (0 == k->tasks[cur->task].wakeup) {
//Wake up. Remove from sleep list and add to priority queue.
                kernel_nd_item *nd = cur;
                cur = cur->next;

                uart_puts("rpi3rtos::kernel_service_sleeping(): Task ");
                uart_u64hex_s(nd->task);
                uart_puts(" is ready to wake up.\n");
//Update queue.
                k->tasks[nd->task].flags &= ~KERNEL_TASK_FLAG_SLEEPING;
                kernel_sleep_task_node_rmv(k, nd->task); //Remove from sleep list.
                kernel_queue_task_node_add(k, nd->task); //Add to queue.
                k->task = k->queue->task;                //Update current task.

                kernel_task_node_list_validate(&k->sleep);

                continue;
            } else if (k->tasks[cur->task].wakeup < 0) {
//Wake up. Remove from sleep list and add to priority queue.
                kernel_nd_item *nd = cur;
                cur = cur->next;

                uart_puts("rpi3rtos::kernel_service_sleeping(): Task ");
                uart_u64hex_s(cur->task);
                uart_puts(" overslept and is ready to wake up.\n");
//Update queue.
                k->tasks[nd->task].flags &= ~KERNEL_TASK_FLAG_SLEEPING;
                k->tasks[nd->task].header->flags |= TASK_HEADER_FLAG_OVERSLEPT;
                kernel_sleep_task_node_rmv(k, nd->task); //Remove from sleep list.
                kernel_queue_task_node_add(k, nd->task); //Add to queue.
                k->task = k->queue->task;                //Update current task.

                kernel_task_node_list_validate(&k->sleep);

                continue;
            } else {
                uart_puts("rpi3rtos::kernel_service_sleeping(): Task ");
                uart_u64hex_s(cur->task);
                uart_puts(" is asleep.\n");
            }
        }
        cur = cur->next;
    }
}

//
//kernel_service_suspended()
// Determine if conditions exist for task resume and if appropriate 
// resume.
//
void kernel_service_suspended(kernel *k) {
    kernel_nd_item *cur = k->suspend.head;

    while(cur) {
        uart_puts("rpi3rtos::kernel_service_suspended(): Servicing suspended task ");
        uart_u64hex_s((u64_t) cur->task);
        uart_puts(".\n");
        kernel_task_flags_print(k->tasks[cur->task].flags);

        if (k->tasks[cur->task].flags & KERNEL_SYSCALL_SUSPEND) {
            if (k->tasks[cur->task].flags & 
                KERNEL_TASK_FLAG_WAKEUP_POST_INIT)
            {
//Suspended after init. Remove from suspend list and add to priority queue.
                uart_puts("rpi3rtos::kernel_service_suspended(): Ready to wake up.\n");

                kernel_nd_item *nd = cur;
                cur = cur->next;
                k->tasks[nd->task].flags &= KERNEL_TASK_FLAG_WAKEUP_CLEAR_ALL;
//Update queue.
                kernel_suspend_task_node_rmv(k, nd->task); //Remove from suspend list.
                kernel_queue_task_node_add(k, nd->task);   //Add to queue.
                k->task = k->queue->task;                  //Update current task.

                kernel_task_node_list_validate(&k->suspend);

                continue;
            }

            if (k->tasks[cur->task].flags & 
                KERNEL_TASK_FLAG_WAKEUP_POST_RESET)
            {
//Suspended after reset. Remove from suspend list and add to priority queue.
                uart_puts("rpi3rtos::kernel_service_suspended(): Ready to wake up.\n");

                kernel_nd_item *nd = cur;
                cur = cur->next;

                k->tasks[nd->task].flags &= KERNEL_TASK_FLAG_WAKEUP_CLEAR_ALL;
//Update queue.
                kernel_suspend_task_node_rmv(k, nd->task); //Remove from suspend list.
                kernel_queue_task_node_add(k, nd->task);   //Add to queue.
                k->task = k->queue->task;                  //Update current task.

                kernel_task_node_list_validate(&k->suspend);

                continue;
            }
        }
        cur = cur->next;
    }
}

void kernel_service_syscall(kernel *k) {
    switch(k->syscall) {
        case 0: //No pending syscall. Done.
        break;

        case KERNEL_SYSCALL_SLEEP:
            uart_puts("rpi3rtos::kernel_service_syscall(): Task ");
            uart_u64hex_s(k->task);
            uart_puts(" is requesting sleep...\n");
            kernel_queue_task_sleep_and_update(k, k->task);
        break;

        case KERNEL_SYSCALL_SUSPEND:
            uart_puts("rpi3rtos::kernel_service_syscall(): Task ");
            uart_u64hex_s(k->task);
            uart_puts(" is requesting suspend...\n");
            kernel_queue_task_suspend_and_update(k, k->task);
        break;

        case KERNEL_SYSCALL_PRIORITY:
            uart_puts("rpi3rtos::kernel_service_syscall(): Task ");
            uart_u64hex_s(k->task);
            uart_puts(" is requesting a priority change...\n");

            uart_puts("rpi3rtos::kernel_service_syscall(): Change priority from ");
            uart_u64hex_s(k->tasks[k->task].priority);
            uart_puts(" to ");
            uart_u64hex_s(k->sysarg.value);
            uart_puts(".\n");

            if (k->sysarg.lo) {
                if (k->tasks[k->task].priority != k->sysarg.lo) {
//Change task priority.
                    uart_puts("rpi3rtos::kernel_service_syscall(): Update priority in queue.\n");
                    k->tasks[k->task].priority = (u64_t) k->sysarg.lo;
                    kernel_queue_task_node_pos_update(k, k->task);
                    k->task = k->queue->task;
                }
            }

            if (k->sysarg.hi) {
//Change task queue options.
                k->tasks[k->task].flags &= KERNEL_TASK_FLAG_QUEUE_CLEAR_ALL;
                k->tasks[k->task].flags |= k->sysarg.hi;
            }
        break;

        default:
            uart_puts("rpi3rtos::kernel_service_syscall(): Task ");
            uart_u64hex_s(k->task);
            uart_puts(" requested unrecognized syscall ");
            uart_u64hex_s(k->syscall);            
            uart_puts("Panic.\n");
            kernel_panic();
        break;
    };

//Clear syscall.
    k->syscall = 0;
    k->sysarg.value = 0;
}

void kernel_service_tick(kernel *k) {
//Update current task. If no tasks on queue then current is kernel.
    if (k->queue) {
        if (k->queue->task == k->task) {
            if (k->tasks[k->queue->task].flags & 
                KERNEL_TASK_FLAG_QUEUE_ROUND_ROBIN) 
            {
//This should move the current task to the back of the list of tasks
//with the same priority. Easy way to do round-robin prioritizing.
                kernel_queue_task_node_pos_update(k, k->task);
            }
        }
        k->task = k->queue->task;
    } else {
        k->task = 0;
    }
}


void kernel_main(kernel *k) {
    u64_t base = task_get_base_addr(0);

    uart_puts("rpi3rtos::kernel_main(): Entering kernel_main(");
    uart_u64hex_s((u64_t) k);
    uart_puts(").\n");

//Set exception handlers for EL1 in hardware.
    uart_puts("rpi3rtos::kernel_main(): Rebased __exception_vectors_start: ");
    uart_u64hex_s((u64_t) __exception_vectors_start + base);
    uart_puts("\n");
    asm volatile ("msr  vbar_el1, %0\n" :: "r"(__exception_vectors_start + base) :);

//Set hardware timer for time slices.
    uart_puts("rpi3rtos::kernel_main(): Setting slice timer...\n");
    irq_disable();
    timer_init_core0(KERNEL_TICK_DURATION_MS);
    irq_enable();

    uart_puts("rpi3rtos::kernel_main(): Done setting slice timer.\n");
    uart_puts("rpi3rtos::kernel_main(): Entering main kernel loop.\n");

//Main kernel loop.
    while(1) {
//
//When an exception or interrupt occurs, context is switched from running
//task to here.
//

//Enter critical section.
        irq_disable();

//Service the suspended tasks.
        kernel_service_suspended(k);

        if (k->ticks > 0) {
//Always service sleeping before syscalls to avoid premature wakeups.
            kernel_service_sleeping(k);
//Tick has elapsed. Service priority queue.
            kernel_service_tick(k);
//Reset tick counter.
            k->ticks = 0;
        }

//Handle pending syscalls (Suspend, Sleep, Priority, Wakeup)
        kernel_service_syscall(k);

        irq_enable();
//Left critical section.

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
