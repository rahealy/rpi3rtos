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

#ifndef MMU_H
#define MMU_H

/*
Memory Layout
Addresses relative.

Task Memory
 Each task is assigned 4MB of contiguous address space laid out as follows:

 0x00200000-0x00400000: Mixed executable R/O and nonexecutable R/W in 4kB blocks.
 0x00000000-0x00200000: Non-executable R/W single 2MB block.
 
 Task executable code is placed at top of address space. Stack grows down. Example:
 
 0x00400000-0x003F8000: Read only executable code takes up 8 4kB blocks.
 0x003F8000-0x00000000: Task BSS + data + stack takes up remaining 4kB and single 2MB block.
*/


//
//Enable the MMU using a list containing number of 4kB R/O blocks for
//each address space.
//
int mmu_enable(u64_t *rolst, u64_t lstsz);

#endif
