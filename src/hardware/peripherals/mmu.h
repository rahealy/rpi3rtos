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

#include "platform.h"

//
//MMU_TASK_MEMORY_SZ
// Amount of memory allocated to a task in 2MB increments. Executable
// size is fixed to 2MB or less. Making this value larger will increase
// the amount of stack space available to the task. It will not increase
// the amount of memory to store the executable code.
//
#define MMU_TASK_MEMORY_SZ      0x00400000

/*
Memory Layout
Each task is assigned 4MB of contiguous address space. Laid out as follows

0x00000000-0x00200000 RTOS Kernel is Task 0
0x00200000-0x00400000 Task 1
0x00400000-0x00800000 Task 2
...

Task Memory
 Each task is assigned 4MB of contiguous address space laid out as follows
 (relative addresses):

 0x00000000-0x00200000: Non-executable R/W single 2MB block.
 0x00200000-0x00400000: Mixed executable R/O and nonexecutable R/W in 4kB blocks.
 
 Task executable code is placed at bottom of address space. Stack grows down. Example:
 
 0x00000000-0x003F8000: Stack starts at 0x003F8000 and grows to 0x00000000
 0x003F8000-0x00400000: Executable task code takes up 8 4kB blocks.
 
 Task executable must not exceed 2MB in size.
*/

typedef u64_t mmu_range_lst [RTOS_MAX_TASKS][2];

//
//Enable the MMU for tasks setting memory containing code as executable 
//read-only.
//
// rolst - Initialized array of read only begin and read only length in
//         bytes for each task.
// numtasks - number of tasks in rolenlst.
//
void mmu_enable(mmu_range_lst rolst, u64_t numtasks);

#endif
