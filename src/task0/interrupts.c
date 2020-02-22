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

void current_el0_irq(void) {
}

void current_el0_fiq(void) {
}

void current_elx_irq(void) {
    if (*TIMER_CTL_AND_STATUS & TIMER_CTL_AND_STATUS_INT_FLG) {
        timer_clr_and_reload();
        uart_puts("current_elx_irq(): Timer tick!\n");
    }
}

void current_elx_fiq(void) {
}

void lower_aarch64_irq(void) {
    uart_puts("lower_aarch64_irq(): HERE!\n");
}

void lower_aarch64_fiq(void) {
}

void lower_aarch32_irq(void) {
}

void lower_aarch32_fiq(void) {
}

