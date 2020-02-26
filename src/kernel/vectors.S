//
//  MIT License
//
//  Copyright (c) 2018-2019 Andre Richter <andre.o.richter@gmail.com>
//
//  Edited by Richard Healy
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//

/*
    Handlers must be 32 instructions or fewer.
    general-purpose registers X0-X30.
    Advanced SIMD and Floating-point registers V0 - V31.
    Some status registers.
    TTBR0_EL1 and TTBR0.
    Thread Process ID (TPIDxxx) Registers.
    Address Space ID (ASID).
*/

.extern task0_header
.extern task0_kernel

.macro SAVE_CONTEXT 
//Save registers on current context stack.
    sub     sp,  sp,  #16 * 16       //Allocate 256 bytes on the stack.
    stp     x0,  x1,  [sp, #16 * 0]
    stp     x2,  x3,  [sp, #16 * 1]
    stp     x4,  x5,  [sp, #16 * 2]
    stp     x6,  x7,  [sp, #16 * 3]
    stp     x8,  x9,  [sp, #16 * 4]
    stp     x10, x11, [sp, #16 * 5]
    stp     x12, x13, [sp, #16 * 6]
    stp     x14, x15, [sp, #16 * 7]
    stp     x16, x17, [sp, #16 * 8]
    stp     x18, x19, [sp, #16 * 9]
    stp     x20, x21, [sp, #16 * 10]
    stp     x22, x23, [sp, #16 * 11]
    stp     x24, x25, [sp, #16 * 12]
    stp     x26, x27, [sp, #16 * 13]
    stp     x28, x29, [sp, #16 * 14]
    mrs     x4,  ELR_EL1             //Exception return address. X0-X3 might contain args. Use x4.
    stp     x4,  x30, [sp, #16 * 15] //Save ELR_EL1 & link register (x30) on current stack.
.endm

.macro RESTORE_CONTEXT 
    ldp     x0, x30,  [sp, #16 * 15]
    msr     ELR_EL1,  x0
    ldp     x0,  x1,  [sp, #16 * 0]
    ldp     x2,  x3,  [sp, #16 * 1]
    ldp     x4,  x5,  [sp, #16 * 2]
    ldp     x6,  x7,  [sp, #16 * 3]
    ldp     x8,  x9,  [sp, #16 * 4]
    ldp     x10, x11, [sp, #16 * 5]
    ldp     x12, x13, [sp, #16 * 6]
    ldp     x14, x15, [sp, #16 * 7]
    ldp     x16, x17, [sp, #16 * 8]
    ldp     x18, x19, [sp, #16 * 9]
    ldp     x20, x21, [sp, #16 * 10]
    ldp     x22, x23, [sp, #16 * 11]
    ldp     x24, x25, [sp, #16 * 12]
    ldp     x26, x27, [sp, #16 * 13]
    ldp     x28, x29, [sp, #16 * 14]
    add     sp,  sp,  #16 * 16
.endm
//ldp     x2, x3, [sp, #16 * 15]  //[x2,x3] = saved ELR_EL1 / link register.
//add     x2, x2, #4              //x2 = increment ELR_EL1 past exception.
//add     x3, x3, #4              //x3 = increment link register past exception.
//stp     x2, x3, [sp, #16 * 15]  //Save ELR_EL1 & link register (x30).

//
//SAVE_CALL_RESTORE_IRQ
// The \handler is expected to return the following:
//  x0 Pointer to the kernel task SP where current task SP will be 
//     saved.
//  x1 If x0 is non-zero the kernel context stack pointer which will be 
//     restored, otherwise ignored.
//
.macro SAVE_CALL_RESTORE_IRQ origin handler
.org \origin
    msr     daifset, #3             //Disable IRQ and FIQ interrupts.
    SAVE_CONTEXT                    //Save context to current stack.
    bl      \handler                //Call handler.
    cbz     x0, 1f                  //Test for context switch. x0 = ptr and x1 = sp.
    mov     x2, sp                  //x2   = Current stack pointer.
    str     x2, [x0]                //[x0] = Stack pointer saved in location passed in x0.
    mov     sp, x1                  //sp   = New stack pointer passed in x1.
1:  b      __restore_context        //Restore and return from exception.
.endm

//
//SAVE_CALL_RESTORE_EXC
// In the case of a syscall x0-x3 may be used as arguments.
//
// The \handler is expected to return the following:
//  x0 Pointer to the kernel task SP where current task SP will be 
//     saved.
//  x1 If x0 is non-zero the kernel context stack pointer which will be 
//     restored, otherwise ignored.
//
.macro SAVE_CALL_RESTORE_EXC origin handler
.org \origin
    msr     daifset, #3             //Disable IRQ and FIQ interrupts.
/*    mrs     x0,  ELR_EL1            //x0 = ELR_EL1
    add     x0, x0, #4              //Increment link register past exception instruction.
    msr     ELR_EL1, x0             //Store modified link register.*/
    SAVE_CONTEXT                    //Save context to current stack.
    bl      \handler                //Call handler.
    cbz     x0, 1f                  //Test for context switch. x0 = ptr and x1 = sp.
    mov     x2, sp                  //x2   = Current stack pointer.
    str     x2, [x0]                //[x0] = Stack pointer saved in location passed in x0.
    mov     sp, x1                  //sp   = New stack pointer passed in x1.
1:  b      __restore_context        //Restore and return from exception.
.endm

// The vector definitions
.section .vectors, "ax"
.align 11
.global __exception_vectors_start
__exception_vectors_start:
    SAVE_CALL_RESTORE_EXC 0x000 current_el0_synchronous   // 0x000
    SAVE_CALL_RESTORE_IRQ 0x080 current_el0_irq           // 0x080
    SAVE_CALL_RESTORE_IRQ 0x100 current_el0_fiq           // 0x100
    SAVE_CALL_RESTORE_EXC 0x180 current_el0_serror        // 0x180

    SAVE_CALL_RESTORE_EXC 0x200 current_elx_synchronous   // 0x200
    SAVE_CALL_RESTORE_IRQ 0x280 current_elx_irq           // 0x280
    SAVE_CALL_RESTORE_IRQ 0x300 current_elx_fiq           // 0x300
    SAVE_CALL_RESTORE_EXC 0x380 current_elx_serror        // 0x380

    SAVE_CALL_RESTORE_EXC 0x400 lower_aarch64_synchronous // 0x400
    SAVE_CALL_RESTORE_IRQ 0x480 lower_aarch64_irq         // 0x480
    SAVE_CALL_RESTORE_IRQ 0x500 lower_aarch64_fiq         // 0x500
    SAVE_CALL_RESTORE_EXC 0x580 lower_aarch64_serror      // 0x580

    SAVE_CALL_RESTORE_EXC 0x600 lower_aarch32_synchronous // 0x600
    SAVE_CALL_RESTORE_IRQ 0x680 lower_aarch32_irq         // 0x680
    SAVE_CALL_RESTORE_IRQ 0x700 lower_aarch32_fiq         // 0x700
    SAVE_CALL_RESTORE_EXC 0x780 lower_aarch32_serror      // 0x780

__restore_context:
    RESTORE_CONTEXT
    msr     daifclr, #3         //Enable IRQ and FIQ interrupts.
    eret

.macro TASK_CONTEXT_SAVE
    sub     sp,  sp,  #16 * 16      //Allocate 256 bytes on the stack.
    stp     x0,  x1,  [sp, #16 * 0]
    stp     x2,  x3,  [sp, #16 * 1]
    stp     x4,  x5,  [sp, #16 * 2]
    stp     x6,  x7,  [sp, #16 * 3]
    stp     x8,  x9,  [sp, #16 * 4]
    stp     x10, x11, [sp, #16 * 5]
    stp     x12, x13, [sp, #16 * 6]
    stp     x14, x15, [sp, #16 * 7]
    stp     x16, x17, [sp, #16 * 8]
    stp     x18, x19, [sp, #16 * 9]
    stp     x20, x21, [sp, #16 * 10]
    stp     x22, x23, [sp, #16 * 11]
    stp     x24, x25, [sp, #16 * 12]
    stp     x26, x27, [sp, #16 * 13]
    stp     x28, x29, [sp, #16 * 14]
    stp     x30, x30, [sp, #16 * 15] //Save link register (x30) twice on current stack.
                                     //This will set ELR_EL1 to LR in __restore_context.
.endm

//
//__task_context_save_and_branch()
// Do not use in an exception or interrupt handler!
//
// x0 Pointer where current saved task context stack pointer will be saved.
// x1 Stack pointer of next task context.
// x2 Address to branch to.
//
.global __task_context_save_and_branch
__task_context_save_and_branch:
    msr     daifset, #3              //Disable IRQ and FIQ interrupts.
    TASK_CONTEXT_SAVE                //Save context.
    mov     x3, sp                   //x3   = Current stack pointer.
    //FIXME: Pointer may not be local to context.
    str     x3, [x0]                 //[x0] = Stack pointer saved in memory pointed at by x0.
    mov     sp, x1                   //sp   = New stack pointer passed in x1.
    msr     daifclr, #3              //Enable IRQ and FIQ interrupts.
    blr     x2                       //Call function supplied in x2.
1:  wfe                              //Trap.
    b       1b

//
//__task_context_save_and_switch()
// Do not use in an exception or interrupt handler!
//
// x0 Pointer where current saved task context stack pointer will be 
//    saved.
// x1 Stack pointer of next task context.
//
.global __task_context_save_and_switch
__task_context_save_and_switch:
    msr     daifset, #3             //Disable IRQ and FIQ interrupts.
    TASK_CONTEXT_SAVE               //Save current context.
    mov     x3, sp                  //x3   = Current stack pointer.
    str     x3, [x0]                //[x0] = Stack pointer saved in location passed in x0.
    mov     sp, x1                  //sp   = New stack pointer passed in x1.
    RESTORE_CONTEXT                 //Restore context.
    msr     daifclr, #3             //Enable IRQ and FIQ interrupts.
    ret                             //Return to whatever was in context link register.
    
