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
//platform.h
// Platform specific defines.
//

#ifndef PLATFORM_H
#define PLATFORM_H

//
// No stdlib so define these ourselves.
//
typedef long long unsigned int u64_t;
typedef unsigned int        u32_t;
typedef unsigned short int  u16_t;
typedef unsigned char       u8_t;

typedef long long int       i64_t;
typedef int                 i32_t;
typedef short int           i16_t;
typedef char                c8_t;

typedef float               f32_t;
typedef double              f64_t;

//
//Maximum number of allowed tasks.
//
#define RTOS_MAX_TASKS 8

//
//Code execution begins here.
//
#define ENTRY_POINT 0x00080000 //Code begins execution here.

//
//The stack spans between end of read only code area and 4MB boundary.
//
#define STACK_START 0x00800000 //Stack decrements from 8MB boundary.


/**********************************************************************
 * MMIO
 * 
 * RPi I/O peripherals are mapped to memory. Define memory locations
 * for peripheral register access.
 *
 *********************************************************************/

//
//Memory mapped I/O Peripherals start here.
//
#define MMIO_BASE 0x3F000000 //Peripheral access starts at 1GB boundary.

//
//Alternative function select register for GPIO
//
#define GPFSEL_OFFSET 0x00200000
#define GPFSEL_0_BASE (MMIO_BASE  + GPFSEL_OFFSET)
#define GPFSEL_1_BASE (GPFSEL_0_BASE + sizeof(u32_t))

#define GPFSEL_0_REG ((volatile u32_t*) GPFSEL_0_BASE)
#define GPFSEL_1_REG ((volatile u32_t*) GPFSEL_1_BASE)

//
//GPPUD GPIO pin clock enable
//
#define GPPUD_OFFSET 0x00200094
#define GPPUD_BASE (MMIO_BASE + GPPUD_OFFSET)

#define GPPUD_REG ((volatile u32_t*) GPPUD_BASE)

//
//GPPUDCLK GPIO pin clock enable
//
#define GPPUDCLK_OFFSET 0x00200098
#define GPPUDCLK_BASE (MMIO_BASE + GPPUDCLK_OFFSET)

#define GPPUDCLK_REG ((volatile u32_t*) GPPUDCLK_BASE)

#endif
