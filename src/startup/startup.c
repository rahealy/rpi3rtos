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

#include "platform.h"
#include "peripherals.h"
#include "task.h"

extern int __bss_end;


#define STARTUP_LIST_HEADER_MAGIC      0x214F4F4D //"MOO!"

//
//startup_header
// The startup image contains executable startup code followed by a list of
// executable task code. Task code is loaded last first.
//
// magic - always ASCII 'TASK'
//
typedef struct _startup_list_header {
    u64_t magic;
} startup_list_header;


//
//startup_panic()
// Infinite loop.
//
void startup_panic() {
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
//curitem Points at current task list item header.
//task    Current task.
//
void startup_load_task_list(task_list_item *curitem, u64_t task) {
    if (TASK_LIST_ITEM_MAGIC != curitem->magic) {
        uart_puts("rpi3rtos::startup_load_task_list(): No task list item found at ");
        uart_u64hex((u64_t) curitem);
        uart_puts(". Panic.\n");
        startup_panic();
    }

    if (0 == curitem->sz) {
        uart_puts("rpi3rtos::startup_load_task_list(): Found end of list at ");
        uart_u64hex((u64_t) curitem);
        uart_puts(".\n");
        return;
    } else {
        u64_t i;
        char *src;
        char *dst;

        //Re-entrant. Seek end of list.
        startup_load_task_list(
            (task_list_item *) ((u64_t)(curitem + 1) + curitem->sz),
            task + 1
        );

        //Task executable data follows the list structure in the image.
        src = (char *) curitem; //(curitem + 1);

        dst = (char *) task_get_base_addr(task);

        uart_puts("rpi3rtos::startup_load_task_list(): Loading task ");
        uart_u64hex_s(task);
        uart_puts(" from image offset ");
        uart_u64hex_s((u64_t) src);
        uart_puts(" to memory offset ");
        uart_u64hex_s((u64_t) dst);
        uart_puts(" (");
        uart_u64hex_s((u64_t) curitem->sz);
        uart_puts(" bytes)");
        uart_puts("\n");

        for(i = 0; i < curitem->sz; ++i) {
            *dst = *src;
            ++src;
            ++dst;
        }

        task_header_rebase(task);
    }
}

//
//startup()
// Load tasks into memory and jump to Task0.
//
void startup(void) {
    //Pointer to first byte in the kernel image after startup.
    startup_list_header *lsthdr = (startup_list_header *) &__bss_end;
    task_header *task0 = task_get_header(0);

    if (-1 == uart_init()) {
        startup_panic();
    }

    if (STARTUP_LIST_HEADER_MAGIC != lsthdr->magic) {
        uart_puts("rpi3rtos::startup(): No startup list header found. Panic.\n");
        startup_panic();
    }

    uart_puts("rpi3rtos::startup(): Found startup list header. Loading tasks from list...\n");

    startup_load_task_list((task_list_item *) (lsthdr + 1), 0);

    uart_puts("rpi3rtos::startup(): Calling task0->start() (");
    uart_u64hex_s((u64_t) task0);
    uart_puts(" ");
    uart_u64hex_s((u64_t) task0->start);
    uart_puts(")\n");

    asm("nop":::);
    task0->start();
    asm("nop":::);

    while(1) {
        asm("wfe":::);
    }
}

static startup_list_header startuplistheader
    __attribute__ ((section (".startup_list_header"))) 
    __attribute__ ((__used__)) = {
    STARTUP_LIST_HEADER_MAGIC
};
