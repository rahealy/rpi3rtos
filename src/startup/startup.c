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
extern int __startup_list_header;

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

void startup_output_platform_sizes() {
    uart_puts("sizeof(int): ");
    uart_u64hex_s(sizeof(int));
    uart_puts("\n");

    uart_puts("sizeof(long int): ");
    uart_u64hex_s(sizeof(long int));
    uart_puts("\n");

    uart_puts("sizeof(long long int): ");
    uart_u64hex_s(sizeof(long long int));
    uart_puts("\n");

    uart_puts("sizeof(float): ");
    uart_u64hex_s(sizeof(float));
    uart_puts("\n");

    uart_puts("sizeof(double): ");
    uart_u64hex_s(sizeof(double));
    uart_puts("\n");
}

void startup_test_floats(void) {
    f64_t f1 = 5000.1;
    f64_t f2 = 0.5;
    
    uart_puts("f1: ");
    uart_u64hex_s((u64_t) f1);
    uart_puts("\n");

    uart_puts("f2: ");
    uart_u64hex_s((u64_t) f2);
    uart_puts("\n");
    
    f1 = f1 - f2;

    uart_puts("f1 - f2: ");
    uart_u64hex_s((u64_t) f1);
    uart_puts("\n");
}

//
//startup_load_task_list()
// Reentrant function loads tasks from the executable image into their
// memory locations. Tasks are loaded from last to first and executable
// image is overwritten bottom to top.
//
//curitem Points at current task list item header.
//task    Current task.
//
u64_t startup_load_task_list(task_list_item *curitem, u64_t task) {
    if (TASK_LIST_ITEM_MAGIC != curitem->magic) {
        uart_puts("rpi3rtos::startup_load_task_list(): No task list item found at ");
        uart_u64hex_s((u64_t) curitem);
        uart_puts(". Panic.\n");
        startup_panic();
    }

    if (0 == curitem->ro_end) {
        uart_puts("rpi3rtos::startup_load_task_list(): Found list item at ");
        uart_u64hex_s((u64_t) curitem);
        uart_puts("\n");
        uart_puts("rpi3rtos::startup_load_task_list(): End of list. Begin loading.\n");
        return task;
    } else {
        u64_t i, cnt;
        char *src;
        char *dst;

        uart_puts("rpi3rtos::startup_load_task_list(): Found list item at ");
        uart_u64hex_s((u64_t) curitem);
        uart_puts(":\n");
        
        uart_puts("rpi3rtos::startup_load_task_list(): ro_end - ");
        uart_u64hex_s((u64_t) curitem->ro_end);
        uart_puts("\n");

        uart_puts("rpi3rtos::startup_load_task_list(): rw_beg - ");
        uart_u64hex_s((u64_t) curitem->rw_beg);
        uart_puts("\n");

        uart_puts("rpi3rtos::startup_load_task_list(): rw_end - ");
        uart_u64hex_s((u64_t) curitem->rw_end);
        uart_puts("\n");

        uart_puts("rpi3rtos::startup_load_task_list(): bss_beg - ");
        uart_u64hex_s((u64_t) curitem->bss_beg);
        uart_puts("\n");

        uart_puts("rpi3rtos::startup_load_task_list(): bss_end - ");
        uart_u64hex_s((u64_t) curitem->bss_end);
        uart_puts("\n");

        uart_puts("rpi3rtos::startup_load_task_list(): Seeking next list item at ");
        uart_u64hex_s((u64_t) curitem + curitem->rw_end);
        uart_puts("...\n");

        //Re-entrant. Seek end of list.
        cnt = startup_load_task_list (
            (task_list_item *) ((u64_t) curitem + curitem->rw_end),
            task + 1
        );

        uart_puts("rpi3rtos::startup_load_task_list(): Loading task ");
        uart_u64hex_s(task);
        uart_puts(" image.\n");

        //Task executable data follows the list structure in the image.
        src = (char *) curitem; //Load the task_list_header too.
        dst = (char *) task_get_base_addr(task);

        uart_puts("rpi3rtos::startup_load_task_list(): Loading read-only segment\n");
        uart_puts("rpi3rtos::startup_load_task_list(): ");
        uart_u64hex_s((u64_t) src);
        uart_puts("->");
        uart_u64hex_s((u64_t) dst);
        uart_puts("(");
        uart_u64hex_s(curitem->ro_end);
        uart_puts(" Bytes)\n");

        for(i = 0; i < curitem->ro_end; ++i) {
            *dst = *src;
            ++src;
            ++dst;
        }

        src = (char *) ((u64_t) curitem + curitem->rw_beg);
        dst = (char *) (task_get_base_addr(task) + curitem->rw_beg);

        uart_puts("rpi3rtos::startup_load_task_list(): Loading read-write segment\n");
        uart_puts("rpi3rtos::startup_load_task_list(): ");
        uart_u64hex_s((u64_t) src);
        uart_puts("->");
        uart_u64hex_s((u64_t) dst);
        uart_puts("(");
        uart_u64hex_s(curitem->rw_end - curitem->rw_beg);
        uart_puts(" Bytes)\n");

        for(i = 0; i < curitem->rw_end - curitem->rw_beg; ++i) {
            *dst = *src;
            ++src;
            ++dst;
        }

        task_header_rebase(task);
        task_bss_zero(task);

        return cnt;
    }
}

//
//startup()
// Load tasks into memory and jump to Task0.
//
void startup(void) {
    //Pointer to first byte in the kernel image after startup.
    startup_list_header *lsthdr = (startup_list_header *) &__startup_list_header;
    task_header *task0;
    u64_t num_tasks;

    if (-1 == uart_init()) {
        startup_panic();
    }

    startup_output_platform_sizes();
    startup_test_floats();
    
    if (STARTUP_LIST_HEADER_MAGIC != lsthdr->magic) {
        uart_puts("rpi3rtos::startup(): No startup list header found at ");
        uart_u64hex_s((u64_t) lsthdr);
        uart_puts(". Panic.\n");
        startup_panic();
    }

    uart_puts("rpi3rtos::startup(): Found startup list header at ");
    uart_u64hex_s((u64_t) lsthdr);
    uart_puts(".\n");
    uart_puts("rpi3rtos::startup(): Loading tasks from list at ");
    uart_u64hex_s((u64_t) lsthdr + sizeof(startup_list_header));
    uart_puts("...\n");

    num_tasks = startup_load_task_list (
        (task_list_item *) ((u64_t) lsthdr + sizeof(startup_list_header)), 0
    );

//Task0 is the kernel. Set stack pointer and branch.
    task0 = task_get_header(0);
    asm ("mov sp, %0" :: "r"(task_get_base_addr(0)) : );
    task0->init(num_tasks);

//task0->start() should never return but just in case.
    uart_puts("rpi3rtos::startup(): task0->main() unexpectedly returned. Panic.\n");
    startup_panic();
}

static startup_list_header startuplistheader
    __attribute__ ((section (".startup_list_header"))) 
    __attribute__ ((__used__)) = {
    STARTUP_LIST_HEADER_MAGIC
};
