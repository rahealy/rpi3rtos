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

#include "timer.h"

//
//Initialize timer for use with core0.
//
void timer_init_core0(u64_t msecs) {
    //Route timer to core 0;
    *TIMER_ROUTE_IRQ = 0x0;
    
    //Enable local timer.
    //Clock is 38,400,000 MHz. Bit 28 enables timer. Bit 29 enables interrupt.
    *TIMER_CTL_AND_STATUS = 0x30000000 + (u64_t)(38400000.0 * ((f64_t) msecs / 1000.0));

    //Set bit 2 to enable non-secure IRQ "CNTPNSIRQ"
    *TIMER_IRQCTL_CORE0 = 0b0010;

    //Clear and reload timer.
    timer_clr_and_reload();    
}

void timer_one_shot_sys(u64_t timer, u64_t msecs) {
    u32_t tm = TIMER_REG_BLK_SYS->CLO + msecs;
    if (1 == timer) {
        TIMER_REG_BLK_SYS->C1 = tm;
        TIMER_REG_BLK_SYS->CS = 0b0010;
    } else {
        TIMER_REG_BLK_SYS->C3 = tm;
        TIMER_REG_BLK_SYS->CS = 0b1000;
    }
}

void timer_delay_sys(u64_t timer, u64_t msecs) {
    u32_t tm = TIMER_REG_BLK_SYS->CLO + msecs;
    if (1 == timer) {
        TIMER_REG_BLK_SYS->C1 = tm;
        TIMER_REG_BLK_SYS->CS = 0b0010;
        while (0 == (TIMER_REG_BLK_SYS->CS & 0b0010)) {}
    } else {
        TIMER_REG_BLK_SYS->C3 = tm;
        TIMER_REG_BLK_SYS->CS = 0b1000;
        while (0 == (TIMER_REG_BLK_SYS->CS & 0b1000)) {}
    }
}
