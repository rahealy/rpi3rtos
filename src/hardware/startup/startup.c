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

#include "startup.h"
#include "uart.h"
#include "mmu.h"

extern int __bss_end;

static startup_task_list flag
    __attribute__ ((section (".task_list"))) 
    __attribute__ ((__used__)) = {
    STARTUP_TASK_LIST_MAGIC, 
    1,          //Only one task for now.
    (u64_t) &__bss_end + sizeof(startup_task_list)
};

void panic() {
    while(1) {
        asm("wfe":::);
    }
}

//
//startup_load_task_list()
// Reentrant function loads tasks from the executable image into their
// memory locations. Tasks are loaded from last to first and image is
// overwritten as they go.
//
//cnt   Number of remaining tasks.
//offst Byte offset into the image of the current task list item header.
//
void startup_load_task_list(u64_t offst, u64_t cnt) {
    if(cnt) {
        u64_t i;
        char *src, *dst;
        startup_task_list_item *item;

        item = (startup_task_list_item *) offst;

        if (STARTUP_TASK_LIST_ITEM_MAGIC != item->magic) {
            uart_puts("rpi3rtos::startup_load_task_item(): No task list item found. Panic.\n");
            panic();
        }

        //Task executable data follows the list structure in the image.
        src = (char *) ++item;

        //Task size is rounded up to the nearest 4kB boundary and loaded
        //at the bottom of the task's address space.
        dst = (char *) ((cnt * MMU_TASK_MEMORY_SZ) - ((item->sz + 0xFFF) / 0x1000));
        
        uart_puts("rpi3rtos::startup_load_task_item(): Loading task ");
        uart_u64hex(cnt - 1);
        uart_puts(" from image offset ");
        uart_u64hex((u64_t) src);
        uart_puts(" to memory offset ");
        uart_u64hex((u64_t) dst);
        uart_puts("\n");

        for(i = 0; i < item->sz; ++i) {
            *dst = *src;
            ++src;
            ++dst;
        }

        startup_load_task_list(item->next, cnt - 1);
    }
}

                       
//
//startup()
// Load tasks into memory and jump to Task0.
//
void startup(void) {
    //Pointer to first byte in the kernel image after startup.
    startup_task_list *lst = (startup_task_list *) &__bss_end;

    if (-1 == uart_init()) {
        panic();
    }

    if (STARTUP_TASK_LIST_MAGIC != lst->magic) {
        uart_puts("rpi3rtos::startup(): No task header found. Panic.\n");
        panic();
    }

    uart_puts("rpi3rtos::startup(): Found task header. Loading tasks...\n");

    startup_load_task_list(lst->head, lst->num_tasks);

    while(1) {
        asm("wfe":::);
    }
}
