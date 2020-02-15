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

void __attribute__((naked)) task_save_context(void) {
    asm(
        "msr    daifset, #3\n"              //Disable IRQ and FIQ interrupts. 
        "sub    sp,  sp,  #16 * 16\n"       //Allocate 256 bytes on the stack.
        "stp    x0,  x1,  [sp, #16 * 0]\n"  //Save registers on the stack.
        "stp    x2,  x3,  [sp, #16 * 1]\n"
        "stp    x4,  x5,  [sp, #16 * 2]\n"
        "stp    x6,  x7,  [sp, #16 * 3]\n"
        "stp    x8,  x9,  [sp, #16 * 4]\n"
        "stp    x10, x11, [sp, #16 * 5]\n"
        "stp    x12, x13, [sp, #16 * 6]\n"
        "stp    x14, x15, [sp, #16 * 7]\n"
        "stp    x16, x17, [sp, #16 * 8]\n"
        "stp    x18, x19, [sp, #16 * 9]\n"
        "stp    x20, x21, [sp, #16 * 10]\n"
        "stp    x22, x23, [sp, #16 * 11]\n"
        "stp    x24, x25, [sp, #16 * 12]\n"
        "stp    x26, x27, [sp, #16 * 13]\n"
        "stp    x28, x29, [sp, #16 * 14]\n"
        "mrs    x0,  ELR_EL1\n"
        "stp    x0,  x30, [sp, #16 * 15]\n"
    );
}

void __attribute__((naked)) task_restore_context(void) {
    asm(
        "ldp    x0, x30,  [sp, #16 * 15]\n"
        "msr    ELR_EL1,  x0\n"
        "ldp    x0,  x1,  [sp, #16 * 0]\n"
        "ldp    x2,  x3,  [sp, #16 * 1]\n"
        "ldp    x4,  x5,  [sp, #16 * 2]\n"
        "ldp    x6,  x7,  [sp, #16 * 3]\n"
        "ldp    x8,  x9,  [sp, #16 * 4]\n"
        "ldp    x10, x11, [sp, #16 * 5]\n"
        "ldp    x12, x13, [sp, #16 * 6]\n"
        "ldp    x14, x15, [sp, #16 * 7]\n"
        "ldp    x16, x17, [sp, #16 * 8]\n"
        "ldp    x18, x19, [sp, #16 * 9]\n"
        "ldp    x20, x21, [sp, #16 * 10]\n"
        "ldp    x22, x23, [sp, #16 * 11]\n"
        "ldp    x24, x25, [sp, #16 * 12]\n"
        "ldp    x26, x27, [sp, #16 * 13]\n"
        "ldp    x28, x29, [sp, #16 * 14]\n"
        "add    sp,  sp,  #16 * 16\n"
        "msr    daifclr, #3\n"
    );
}

void task_save(task_header *th) {
    u64_t sp = 0;

    asm volatile (
        "mov    %0, sp\n"
        "mov    sp, %1\n"
        : "=r"(sp) : "r"(th->sp) : 
    );

    task_save_context();
    
    asm volatile (
        "mov    %0, sp\n"
        "mov    sp, %1\n"
        : "=r"(th->sp) : "r"(sp) : 
    );
}

void task_zero_bss(char *bss, u64_t sz) {
    int i;
    for (i = 0; i < sz; ++i) {
        bss[i] = 0;
    }
}
