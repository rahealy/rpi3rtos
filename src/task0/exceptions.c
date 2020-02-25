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

#include "uart.h"
#include "kernel.h"

extern volatile kernel task0_kernel;

u64_t current_el0_synchronous(void) { return 0; }
u64_t current_el0_serror(void) { return 0; }
u64_t current_elx_synchronous(void) {
    if(1) { 
//FIXME: Test for suspend syscall. Examine ESR_EL1. If EC bits = 0b010101
//FIXME: then ISS contains syscall.
        uart_puts("current_elx_synchronous(): Exception is syscall.\n");
        uart_puts("current_elx_synchronous(): task sp pointer = ");
        uart_u64hex_s((u64_t) &task0_kernel.tasks[task0_kernel.task].sp);
        uart_puts("\n");
//Set syscall.
        task0_kernel.syscall = 0x1;
//x0 (return value) is a pointer to the current stack pointer.
//x1 is the kernel context stack pointer which will be restored.
        asm ("mov x1, %0" :: "r"(task0_kernel.tasks[0].sp) : );
        return (u64_t) &task0_kernel.tasks[task0_kernel.task].sp;
    }
    return 0;
}
u64_t current_elx_serror(void) { return 0; }
u64_t lower_aarch64_synchronous(void) { return 0; }
u64_t lower_aarch64_serror(void) { return 0; }
u64_t lower_aarch32_synchronous(void) { return 0; }
u64_t lower_aarch32_serror(void) { return 0; }
