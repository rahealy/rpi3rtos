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
#include "mbox.h"

//
//Communication with the GPU is through a mailbox mechanism.
//
#define VIDEOCORE_MBOX (MMIO_BASE + 0x0000B880)

#define MBOX_STATUS_FULL        0x80000000
#define MBOX_STATUS_EMPTY       0x40000000
#define MBOX_RESPONSE_SUCCESS   0x80000000
#define MBOX_RESPONSE_ERROR     0x80000001
#define MBOX_REG_BLK            ((volatile mbox_register_block *) VIDEOCORE_MBOX)

typedef struct _mbox_register_block {
    u32_t read;
    u32_t res0[5];
    u32_t status;
    u32_t res1;
    u32_t write;
} mbox_register_block;

int mbox_call(mbox_buf *buf, u64_t ch) {
    u32_t addr;
    u32_t resp;
    
//Wait until mailbox is ready.
    while(MBOX_REG_BLK->status &  MBOX_STATUS_FULL) {
        asm volatile ("nop\n" :::);
    }

//Send buffer. Buffer struct is 16 byte aligned so last 4 bits always 0. 
//Last 4 bits used to indicate channel.
    addr = (u32_t) (((u64_t) buf & (~0xF)) | (ch & 0xF));
    MBOX_REG_BLK->write = addr;

//Wait for response.
    while(1) {
        while(MBOX_REG_BLK->status &  MBOX_STATUS_EMPTY) {
            asm volatile ("nop\n" :::);
        }

        resp = MBOX_REG_BLK->read;
        if (resp == addr) {
//Response to this sent message.
            switch(buf->buffer[1]) {
                case MBOX_RESPONSE_SUCCESS:
                return 0;
                case MBOX_RESPONSE_ERROR:
                return -1;
            }
        }
    }
}
