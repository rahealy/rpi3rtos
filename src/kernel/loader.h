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
RTOS executable file is laid out as follows:

0x00000000 RTOS kernel.
0x0008n000 First task as data.
...
0x00nnnnnn Last task as data.

Tasks are loaded into 4MB address spaces in last first order. Address spaces are 
filled from higher memory address to lower address possibly overwriting already
loaded tasks in the initial executable. Example:

Executable File:
0x00000000 RTOS kernel (16kB for example. May be different.)
0x00004000 Task 1 in ELF format as data (4kB for example. May be different.)
0x00005000 Task 2 in ELF format as data (4kB for example. May be different.)

Memory:
0x00080000 RTOS kernel.
0x00200000 Task 1 Loaded second - bottom to top.
0x00400000 Task 2 Loaded first - bottom to top.

*/

#ifndef LOADER_H
#define LOADER_H

//
//loader_task
// A task is 
typedef struct _loader_task {
} loader_task;

#endif
