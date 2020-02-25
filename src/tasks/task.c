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


//
//task.c
//

#include "task.h"
#include "uart.h"

inline task_list_item *task_get_list_item(u64_t task) {
    task_list_item *li = (task_list_item *) task_get_base_addr(task);
    if (TASK_LIST_ITEM_MAGIC == li->magic) {
        return li;
    } else {
        uart_puts("rpi3rtos::task_get_list_item(): Not header!\n");
        return 0;
    }
}

inline task_header *task_get_header(u64_t task) {
    task_list_item *li = task_get_list_item(task);
    task_header *th    = (task_header *) ((u64_t) li + li->rw_beg);
    if (TASK_HEADER_MAGIC == th->magic) {
        return th;
    } else {
        uart_puts("rpi3rtos::task_get_header(): Not header!\n");
        return 0;
    }
}

//
//task_header_rebase()
// After the task has been loaded into memory rebase the header to
// actual locations rather than relative.
//
void task_header_rebase(u64_t task) {
    task_header *th = task_get_header(task);
    u64_t base = (u64_t) task_get_base_addr(task);

    uart_puts("rpi3rtos::task_header_rebase(): Re-basing task ");
    uart_u64hex_s(task);
    uart_puts(" header to ");
    uart_u64hex_s(base);
    uart_puts("\n");

    uart_puts("rpi3rtos::task_header_rebase(): init() ");
    uart_u64hex_s((u64_t) th->init);
    uart_puts("->");
    uart_u64hex_s((u64_t) th->init + base);
    uart_puts("\n");

    uart_puts("rpi3rtos::task_header_rebase(): reset() ");
    uart_u64hex_s((u64_t) th->reset);
    uart_puts("->");
    uart_u64hex_s((u64_t) th->reset + base);
    uart_puts("\n");

    uart_puts("rpi3rtos::task_header_rebase(): start() ");
    uart_u64hex_s((u64_t) th->start);
    uart_puts("->");
    uart_u64hex_s((u64_t) th->start + base);
    uart_puts("\n");

    th->init  = (taskfn)(((char *) th->init)  + base);
    th->reset = (taskfn)(((char *) th->reset) + base);
    th->start = (taskfn)(((char *) th->start) + base);
}

void task_bss_zero(u64_t task) {
    int i;
    u64_t base = (u64_t) task_get_base_addr(task);
    task_list_item * li = (task_list_item *) base;
    char *bss = (char *) li->bss_beg + base;

    uart_puts("rpi3rtos::task_bss_zero(): ");
    uart_u64hex_s(li->bss_beg + base);
    uart_puts("-");
    uart_u64hex_s(li->bss_end + base);
    uart_puts("\n");

    for (i = 0; i < li->bss_end - li->bss_beg; ++i) {
        bss[i] = 0;
    }
}

void task_suspend() {
    asm("svc    0");
}
