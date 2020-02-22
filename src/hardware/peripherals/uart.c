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
//uart.c
// Uart driver for PL011
//

#include "uart.h"
#include "mbox.h"

#define UART_FSEL14_MASK    0x00007000 //FSEL14 [14:12] = Input (0b000).
#define UART_FSEL15_MASK    0x00038000 //FSEL15 [17:15] = Input (0b000).
#define UART_FSEL14_TXD0    0x00004000 //FSEL14 [14:12] = TXD0 (0b100).
#define UART_FSEL15_RXD0    0x00020000 //FSEL15 [17:15] = RXD0 (0b100).
#define UART_GPPUD_MASK     0x00000003 //GPPUD  [1:0] = 0b00
#define UART_PUDCLK14_MASK  0x00004000 //PUDCLK14 [14:14] = 0b1 
#define UART_PUDCLK15_MASK  0x00008000 //PUDCLK14 [14:14] = 0b1 
#define UART_ICR_MASK       0x000007FF //ICR [10:0] = 0 
#define UART_IBRD_MASK      0x0000FFFF //IBRD [15:0] = 0 
#define UART_FBRD_MASK      0x0000001F //FBRD [5:0]
#define UART_LCRH_8N1       0x00000060 //LCRH [6:5] = 0b11
#define UART_CR_RXE         0x00000200 //RXE [9:9] = 0b1
#define UART_CR_TXE         0x00000100 //TXE [8:8] = 0b1
#define UART_CR_UARTEN      0x00000001 //UARTEN [0:0] = 0b1

//
//UART_FR_TXFF_FLAG
// Bit is set while UART is transmitting.
//
#define UART_FR_TXFF_FLAG 0x00000020 //TXFF [5:5] 

typedef struct _uart0_register_block {
//Data register.
    u32_t DR;       // 0x00
//Undocumented
    u32_t RSRECR;   // 0x04
//Reserved 0
    u32_t res0[4];  // 0x08 - 0x014 not assigned.
//Flag register
    u32_t FR;       // 0x18
//Reserved 1
    u32_t res1;     // 0x1c not assigned.
//Not in use
    u32_t ILPR;     // 0x20
//Integer Baud rate divisor
    u32_t IBRD;     // 0x24
//Fractional Baud rate divisor
    u32_t FBRD;     // 0x28
//Line Control register
    u32_t LCRH;     // 0x2C
//Control register
    u32_t CR;       // 0x30
//Interupt FIFO Level Select Register
    u32_t IFLS;     // 0x34
//Interupt Mask Set Clear Register
    u32_t IMSC;     // 0x38
//Raw Interupt Status Register
    u32_t RIS;      // 0x3C
//Masked Interupt Status Register
    u32_t MIS;      // 0x40
//Interupt Clear Register
    u32_t ICR;      // 0x44
} uart0_register_block;

#define UART_0_REG_BLK ((volatile uart0_register_block*) UART0_BASE)

int uart_init(void) {
    int i;
    mbox_buf buf;

    buf.buffer[0] = 9 * sizeof(u32_t);
    buf.buffer[1] = 0;
    buf.buffer[2] = MBOX_TAG_SETCLKRATE;
    buf.buffer[3] = 12;
    buf.buffer[4] = 8;
    buf.buffer[5] = MBOX_CLOCK_UART;
    buf.buffer[6] = 4000000;
    buf.buffer[7] = 0;
    buf.buffer[8] = 0;

//Memory barrier makes sure all writes to buffer have been committed.
    asm("dmb sy\n":::);
    
//Use Videocore GPU to set clocks.    
    if(-1 == mbox_call(&buf, MBOX_CHANNEL_PROP)) {
        return -1;
    }

//Select UART function for the GPIO pins.
    //Reset to Input
    *GPFSEL_1_REG = *GPFSEL_1_REG & (~UART_FSEL14_MASK); 
    *GPFSEL_1_REG = *GPFSEL_1_REG & (~UART_FSEL15_MASK);

    //Set to TXD0, RXD0 (UART0)
    *GPFSEL_1_REG = *GPFSEL_1_REG | UART_FSEL14_TXD0;    
    *GPFSEL_1_REG = *GPFSEL_1_REG | UART_FSEL15_RXD0;    
    
//Turn off GPIO pull up/down for UART.
    *GPPUD_REG = *GPPUD_REG & (~UART_GPPUD_MASK); //[1:0] = 0b00

    for(i = 0; i < 150; ++i) {
        asm volatile ("nop\n" :::);
    }

//Clock in the new GPIO settings.
    *GPPUDCLK_REG = *GPPUDCLK_REG |
                    UART_PUDCLK14_MASK | 
                    UART_PUDCLK15_MASK;

    for(i = 0; i < 150; ++i) {
        asm volatile ("nop\n" :::);
    }

    *GPPUDCLK_REG = 0x00000000;

//Set up UART.
    //Clear interrupt control reg.
    UART_0_REG_BLK->ICR  = UART_0_REG_BLK->ICR   & (~UART_ICR_MASK);            //[10:0] = 0. 
    //Results in 115200 baud.
    UART_0_REG_BLK->IBRD = (UART_0_REG_BLK->IBRD & (~UART_IBRD_MASK)) | 0x0002; //[15:0] = 0x0002. 
    //Fractional baud rate.
    UART_0_REG_BLK->FBRD = (UART_0_REG_BLK->FBRD & (~UART_FBRD_MASK)) | 0xB;    //[5:0] = 0xB
    //Bit and parity
    UART_0_REG_BLK->LCRH = (UART_0_REG_BLK->LCRH | UART_LCRH_8N1);              //[6:5] = 0b11
    //Enable UART, RX & TX
    UART_0_REG_BLK->CR   = UART_0_REG_BLK->CR |
                 UART_CR_RXE |
                 UART_CR_TXE |
                 UART_CR_UARTEN;

    return 0;
}

void uart_send(char c) {
    //Wait until UART is not transmitting.
    while(UART_0_REG_BLK->FR & UART_FR_TXFF_FLAG) {
        asm volatile ("nop\n" :::);
    }
    //Write character to UART buffer.
    UART_0_REG_BLK->DR = (u32_t) c;
}

void uart_puts(const char *str) {
    const char *ch;
    for (ch = str; *ch != 0x00; ++ch) {
        // convert newline to carrige return + newline
        if ('\n' == *ch) {
            uart_send('\r');
        }
        uart_send(*ch);
    }
}

void uart_nputs(char *str, u32_t numch) {
    int i = 0;
    char *ch = str;
    
    for ( ; (i < numch) && (*ch != 0x00); ++i) {
        // convert newline to carrige return + newline
        if ('\n' == *ch) {
            uart_send('\r');
        }
        uart_send(*ch);
        ++ch;
    }
}

char uart_tohex(u8_t val) {
    switch (val & 0x0F) {
        case 0x0: return '0';
        case 0x1: return '1';
        case 0x2: return '2';
        case 0x3: return '3';
        case 0x4: return '4';
        case 0x5: return '5';
        case 0x6: return '6';
        case 0x7: return '7';
        case 0x8: return '8';
        case 0x9: return '9';
        case 0xA: return 'A';
        case 0xB: return 'B';
        case 0xC: return 'C';
        case 0xD: return 'D';
        case 0xE: return 'E';
        case 0xF: return 'F';
        default:  return '?';
    }
}

void uart_u64hex(u64_t val) {
    int i;
    uart_puts("0x");
    for(i = 15; i > -1; --i) {
        uart_send(
            uart_tohex((u8_t) (val >> i * 4))
        );
    }
}

void uart_u64hex_s(u64_t val) {
    int i, b;
    b = 0;
    u8_t byte;
    uart_puts("0x");
    for(i = 15; i > 0; --i) {
        byte = (u8_t) (val >> i * 4);
        if (0 == byte) {
            if (0 == b) {
                continue;
            }
        } else if (0 == b) {
            b = 1;
        }
        uart_send(uart_tohex(byte));
    }
    uart_send(uart_tohex(val));
}
