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

#include "uart.h"
#include "kernel.h"

//
//ESR_EL1 Exception syndrome register contains information about what
//        triggered the exception.
//
#define EXCEPTIONS_ESR_EL1_EC               0xFC000000       //EC (Exception Class) bits [31:26]
#define EXCEPTIONS_ESR_EL1_EC_AARCH64_SVC   (0b010101 << 26) //Exception was a service call.
#define EXCEPTIONS_ESR_EL1_ISS              0x1FFFFFF        //ISS bits [24:0]

//
//Exception handlers.
//

u64_t current_el0_synchronous(void)     { return 0; }
u64_t current_el0_serror(void)          { return 0; }

void current_elx_synchronous(u64_t arg) {
    u64_t *sp_ptr;
    u64_t sp;
    u64_t esr; //Exception class.

    uart_puts("rpi3rtos::current_elx_synchronous(): An exception has occurred.\n");

//Load the contents of the exception syndrome register.
    asm volatile ( "mrs %0, esr_el1\n" : "=r"(esr) :: );

//Determine what caused the exception.
    switch (esr & EXCEPTIONS_ESR_EL1_EC) {
        case EXCEPTIONS_ESR_EL1_EC_AARCH64_SVC: //Syscall from task.
            sp_ptr = kernel_get_cur_task_sp_ptr();
            sp = kernel_get_sp();

            uart_puts("rpi3rtos::current_elx_synchronous(): "
                      "Exception is a syscall from task ");
            uart_u64hex_s(kernel_get_pointer()->task);
            uart_puts(".\n");

            kernel_get_pointer()->syscall = (esr & EXCEPTIONS_ESR_EL1_ISS); //Syscall number in ISS.
            kernel_get_pointer()->sysarg.value = arg; //Argument passed in x0.

            uart_puts("rpi3rtos::current_elx_synchronous(): Syscall is ");
            uart_u64hex_s(kernel_get_pointer()->syscall);
            uart_puts(".\n");

            uart_puts("rpi3rtos::current_elx_synchronous(): Sysarg is ");
            uart_u64hex_s(kernel_get_pointer()->sysarg.value);
            uart_puts(".\n");

//             uart_puts("rpi3rtos::current_elx_synchronous(): "
//                       "Pointer to current kernel task SP located at ");
//             uart_u64hex_s((u64_t) sp_ptr);
//             uart_puts("\n");
// 
//             uart_puts("rpi3rtos::current_elx_synchronous(): Kernel SP is ");
//             uart_u64hex_s(sp);
//             uart_puts("\n");

            uart_puts("rpi3rtos::current_elx_synchronous(): "
                      "Exception handled. Switching to kernel task.\n");

//
//Exception handler stub expects the following conditions after return:
// x0 Pointer to the kernel task SP where current task SP will be 
//    saved.
// x1 kernel context stack pointer which will be restored.
//
            asm volatile (
                "mov x0, %0\n"
                "mov x1, %1\n" 
                :: "r"(sp_ptr), "r"(sp) :
            );
        return;

        default:
        break;
    }

    uart_puts("current_elx_synchronous(): Exception is not handled. Panic.\n");
    kernel_panic();
}

u64_t current_elx_serror(void)          { return 0; }
u64_t lower_aarch64_synchronous(void)   { return 0; }
u64_t lower_aarch64_serror(void)        { return 0; }
u64_t lower_aarch32_synchronous(void)   { return 0; }
u64_t lower_aarch32_serror(void)        { return 0; }
