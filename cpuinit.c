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

extern void _start(void);
extern void __exception_vectors_start(void);
void main(void);

#define EL_BITS 0x0000000C
//
//_cpuinit()
// Initialize the CPU state and access for EL1, AARCH64.
//
void _cpuinit(void) {
//If at EL2 drop down to EL1
    u64_t reg0 = 0, reg1 = 0;

    asm volatile ("mrs %0, currentel" : "=r"(reg0)::);
    
    if (0x8 == (reg0 & EL_BITS)) { //In EL2. Drop down to EL1
//Set up timers.
        asm volatile (
            "mrs    %0, cnthctl_el2\n"   //Read counter control into variable.
            "orr    %0, %0, #3\n"        //Allow EL1 access to the physical timer & counter
            "msr    cnthctl_el2, %0\n"   //Write updated counter control value.
            "msr    cntvoff_el2, xzr\n"  //Virtual timer offset is 0.
            : "=r"(reg0): "r"(reg0):
        );

//Disable traps to hypervisor (won't be using EL2)
        asm volatile (
            "mov    %0, 0x33ff\n"       //Default 0x33ff is fine.
            "msr    cptr_el2, %0\n"     //Default 0x33ff is fine.
            "msr    hstr_el2, xzr\n"    //Don't trap anything in the hypervisor.
            : : "r"(reg0):
        );

//EL1 will be running in AARCH64 mode.
        asm volatile (
            "mrs    %0, hcr_el2\n"         //Read Hypervisor Configuration Register
            "orr    %0, %0, #0x80000000\n" //Set bit 31 for AARCH64 mode in EL1.
            "msr    hcr_el2, %0\n"         //Write Hypervisor Configuration Register
            : "=r"(reg0): "r"(reg0):
        );

//Set SPSR_EL2
        asm volatile (
            "mov    %0, #0x3C5\n"       //Set D,A,I,F flags. Set return to EL1h.
            "mrs    %1, spsr_el2\n"     //Read Hypervisor Configuration Register
            "orr    %1, %1, %0\n"       //Set flags.
            "msr    spsr_el2, %1\n"     //Write Hypervisor Configuration Register
            : "=r"(reg0), "=r"(reg1)
            : "r"(reg0), "r"(reg1):
        );

//Set exception handlers for EL1.
        asm volatile (
            "msr    vbar_el1, %0\n" :: "r"(__exception_vectors_start) :
        );

//On ERET, execution will jump to location in elr_el2.
        asm volatile (
            "msr    elr_el2, %0\n" :: "r"(main) :
        ); 

//On ERET stack pointer is set to sp_el1
        asm volatile (
            "msr    sp_el1, %0\n" :: "r"(_start) :
        );

//On ERET go to EL1, set stack pointer to sp_el1 and jump to main()
        asm volatile ("eret\n" :::);
    }

    main();
}

