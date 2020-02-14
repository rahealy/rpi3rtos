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
//GPU Mailbox routines.
//

#ifndef MBOX_H
#define MBOX_H

#include "platform.h"

#define MBOX_TAG_SETCLKRATE 0x00038002
#define MBOX_CLOCK_UART     0x00000002
#define MBOX_CHANNEL_PROP   0x00000008

//
//mbox_buf
// Buffer used to share information with the GPU mailbox.
//
typedef struct _mbox_buf {
    u32_t buffer[36];
} __attribute__((packed, aligned(16))) mbox_buf;

//
//Call mailbox with buffer contents.
// Returns: -1 on error. 0 on success.
//
int mbox_call(mbox_buf *mbox, u32_t ch);

#endif
