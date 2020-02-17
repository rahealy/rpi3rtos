//
//  MIT License
//
//  Copyright (c) 2020 Richard Healy 
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//

//
//uart.h
// Uart driver for PL011
//
#ifndef UART_H
#define UART_H

//
//UART0 PL011 registers
//
#define UART0_BASE (MMIO_BASE + 0x00201000)

#include "platform.h"

//
//uart_init()
// Initialze uart.
// Returns: -1 on error, 0 on success.
//
int uart_init(void);

//
//uart_send()
// Send one character over the UART interface.
//
void uart_send(char c);

//
//uart_nputs()
// Sends up to 'numch' characters or null terminated string.
//
void uart_nputs(char *str, u32_t numch);

//
//uart_puts()
// Sends null terminated string.
//
void uart_puts(const char *str);

//
//uart_u64hex()
// Writes 64 bit number as hex.
//
void uart_u64hex(u64_t val);

//
//uart_u64hex_s()
// Writes 64 bit number as hex.
//
void uart_u64hex_s(u64_t val);

#endif
