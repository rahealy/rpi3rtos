/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/*
 * start.S
 *  First code run.
 */

.section ".text.start"

.global _start

_start:
    mrs     x1, mpidr_el1   //Get multiprocessor affinity register
    and     x1, x1, #3      //Mask off bottom 2 bits.
    cbz     x1, _cinit      //If x1 == 0 branch to 2:
1:  wfe                     //Multi-core enabled. Not a valid startup state.
    b       1b              //Loop forever.
_cinit:                     //Multi-core not enabled. Running on core 0. Set up for C.
    ldr     x1,=_start      //Initially set stack to span 0x80000->0x00000 
    mov     sp, x1          //Set stack pointer.
_zbss:                      //Zero the BSS segment.
    ldr     x1, =__bss_beg  //__bss_beg defined in linker script.
    ldr     w2, =__bss_sz   //__bss_sz  "
2:  cbz     w2, _jmp        //No more bytes to zero. Branch.
    str     xzr, [x1], #8   //Write zero to location at X1, 64bit width.
    sub     w2, w2, #1      //Subtract 1 from w2 counter.
    cbnz    w2, 2b          //If counter not zero then branch else fall through.
_jmp:                       //Stack pointer set and bss zeroed. Branch to C function.
    bl      _cpuinit        
    b       1b
