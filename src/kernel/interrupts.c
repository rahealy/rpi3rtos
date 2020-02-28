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
#include "irq.h"
#include "timer.h"
#include "kernel.h"

u64_t current_el0_irq(void) { return 0; }
u64_t current_el0_fiq(void) { return 0; }

void current_elx_irq(void) {
    u64_t *sp_ptr;
    u64_t sp;

    uart_puts("rpi3rtos::current_elx_irq(): An interrupt has occurred.\n");

    sp_ptr = kernel_get_cur_task_sp_ptr();
    sp = kernel_get_sp();

    if (*TIMER_CTL_AND_STATUS & TIMER_CTL_AND_STATUS_INT_FLG) {
        uart_puts("rpi3rtos::current_elx_irq(): Timer has expired. Clear and reload.\n");
        timer_clr_and_reload();
        ++kernel_get_pointer()->ticks;
    }

    uart_puts("rpi3rtos::current_elx_irq(): Pointer to current kernel task SP located at ");
    uart_u64hex_s((u64_t) sp_ptr);
    uart_puts("\n");

    uart_puts("rpi3rtos::current_elx_irq(): Kernel SP is ");
    uart_u64hex_s(sp);
    uart_puts("\n");

    uart_puts("rpi3rtos::current_elx_irq(): Interrupt handled. Switching to kernel task.\n");

//
//IRQ handler stub expects the following conditions after return:
// x0 Pointer to the kernel task SP where current task SP will be 
//    saved.
// x1 kernel context stack pointer which will be restored.
//
    asm volatile (
        "mov x0, %0\n"
        "mov x1, %1\n" 
        :: "r"(sp_ptr), "r"(sp) :
    );

    return;
}

u64_t current_elx_fiq(void) { return 0; }
u64_t lower_aarch64_irq(void) { return 0; }
u64_t lower_aarch64_fiq(void) { return 0; }
u64_t lower_aarch32_irq(void) { return 0; }
u64_t lower_aarch32_fiq(void) { return 0; }
