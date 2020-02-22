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

/*
timer.h
 The RPi3 contains 2 timers. A system timer fixed to 1MhZ and a local ARM core
 timer based on crystal speed which can only be routed to one core at a time.
*/

#ifndef TIMER_H
#define TIMER_H

#include "platform.h"

//
//System Timer
//
#define TIMER_OFFSET_SYS 0x00003000
#define TIMER_BASE_SYS   (MMIO_BASE + TIMER_OFFSET_SYS)

//
//ARM Local Timer
//
#define TIMER_CTL_CORE       ((volatile u32_t *) 0x40000000)
#define TIMER_PRESCALER_CORE ((volatile u32_t *) 0x40000008)
#define TIMER_ROUTE_IRQ      ((volatile u32_t *) 0x40000024)
#define TIMER_CTL_AND_STATUS ((volatile u32_t *) 0x40000034)
#define TIMER_CLR_AND_RELOAD ((volatile u32_t *) 0x40000038)
#define TIMER_IRQCTL_CORE0   ((volatile u32_t *) 0x40000040)
#define TIMER_IRQSRC_CORE0   ((volatile u32_t *) 0x40000060)

#define TIMER_CTL_AND_STATUS_INT_FLG 0x80000000 //R/O bit 31 set when there's an interrupt.

//
//timer_register_block{}
// Register block for the system timer.
//
typedef struct _timer_register_block_sys {
///Control and status.
    u32_t CS;
///Timer counter low 32 bits.
    u32_t CLO;
///Timer counter high 32 bits.
    u32_t CHI;
///Timer compare 0.
    u32_t C0;
///Timer compare 1.
    u32_t C1;
///Timer compare 2.
    u32_t C2;
///Timer compare 3.
    u32_t C3;
} timer_register_block_sys;


#define TIMER_REG_BLK_SYS ((volatile timer_register_block_sys *) TIMER_BASE_SYS)

//
//Initializes local timer for use with core 0.
//
void timer_init_core0(u64_t msecs);

//
//timer_clr_and_reload()
// Clear and reload local timer.
//
inline void timer_clr_and_reload(void) {
//Bit 31 clears interrupt. Bit 30 reloads timer.
    *TIMER_CLR_AND_RELOAD = 0xC0000000;
}

//
//timer_one_shot_sys()
// Non-repeating one shot timer uses the system timer. Returns 
// immediately.
//
void timer_one_shot_sys(u64_t timer, u64_t msecs);

//
//timer_delay_sys()
// Non-repeating one shot timer uses the system timer. Delays 
// for 'msecs' milliseconds then returns.
//
void timer_delay_sys(u64_t timer, u64_t msecs);
#endif

