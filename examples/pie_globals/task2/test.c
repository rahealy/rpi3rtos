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
#include "uart.h"

//
//Global counter variable.
//
int g_counter = 0;

//
//Global counter variable.
//
static int s_counter = 0;

//
//Global pointer variable.
//
int *g_pointer = 0;

//
//Global pointer variable.
//
static int *s_pointer = 0;

void test_print_local(void) {
    int *g_counter_ptr = &g_counter;
    int *s_counter_ptr = &s_counter;

    s_pointer = &s_counter;

    uart_puts("test_print_local(): In test.c\n");

    uart_puts("test_print_local(): &g_counter = ");
    uart_u64hex_s((u64_t) &g_counter);
    uart_puts("\n");

    uart_puts("test_print_local(): g_counter_ptr = ");
    uart_u64hex_s((u64_t) g_counter_ptr);
    uart_puts("\n");

    uart_puts("test_print_local(): &g_pointer = ");
    uart_u64hex_s((u64_t) &g_pointer);
    uart_puts("\n");    

    uart_puts("test_print_local(): &s_counter = ");
    uart_u64hex_s((u64_t) &s_counter);
    uart_puts("\n");

    uart_puts("test_print_local(): s_counter_ptr = ");
    uart_u64hex_s((u64_t) s_counter_ptr);
    uart_puts("\n");

    uart_puts("test_print_local(): &s_pointer = ");
    uart_u64hex_s((u64_t) &s_pointer);
    uart_puts("\n");    
}
