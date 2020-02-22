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

#ifndef IRQ_H
#define IRQ_H

#include "platform.h"

#define IRQ_OFFSET 0x0000B000
#define IRQ_BASE (MMIO_BASE + IRQ_OFFSET + 0x200)

typedef struct _irq_register_block {
    u32_t BASIC_PENDING;
    u32_t PENDING_1;
    u32_t PENDING_2;
    u32_t FIQ;
    u32_t ENABLE_1;
    u32_t ENABLE_2;
    u32_t ENABLE_BASIC;
    u32_t DISABLE_1;
    u32_t DISABLE_2;
    u32_t DISABLE_BASIC;
} irq_register_block;

#define IRQ_REG_BLK ((irq_register_block *) IRQ_BASE)

inline void irq_enable(void) {
    asm volatile ("msr  daifclr, #2\n");
}

inline void irq_disable(void) {
    asm volatile ("msr  daifset, #2\n");
}

//
//irq_enable_system_timer()
// Enable system timer 1 or 3 interrupts
//
void irq_enable_system_timer(u64_t timer);

#endif
