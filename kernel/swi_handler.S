@ swi_handler.S: SWI Handler
@
@ Author: Harry Q Bovik <hqbovik@andrew.cmu.edu>
@ Date:   Tue, 23 Oct 2007 11:20:33 -0400
@
@ Hint: To change the SWI handler in class, we need to accommodate a return
@   value from the C_SWI_Handler.

    .extern C_SWI_Handler
    .extern print_int

    #include <arm/psr.h>

    .file   "swi_handler.S"
    .text

    .global swi_handler
swi_handler:
    sub     sp, sp, #4
    stmfd   sp!, {r0-r12, lr}

    @ get spsr into general purpose (gp) registers
    mrs     r2, spsr

    @ store spsr above gp registers
    str     r2, [sp, #14*4]

    @ Enables irqs
    mrs     r2, cpsr
    orr     r2, r2, #PSR_FIQ  @ Disables fiqs
    bic     r2, r2, #PSR_IRQ  @ Enables irqs
    msr     cpsr, r2

    @ restores r8 function table
    ldr     r8, =global_data
    ldr     r8, [r8]

    @ Set Param 1: extract SWI and get SWI no. by bit-masking
    ldr     r0, [lr, #-4]
    bic     r0, r0, #0xff000000

    @ Set Param 2: pointer to parameters on stack
    mov     r1, sp

    @ go to handler
    bl      C_SWI_Handler

    @ restore spsr
    ldr     r2, [sp, #14*4]
    msr     spsr, r2

    stmfd sp!, {r0}
    mrs r0, cpsr
    bl print_int
    ldmfd sp!, {r0}

    @ Discard the r0 on the stack (r0 has ret value).
    add     sp, sp, #4
    ldmfd   sp!, {r1-r12, lr}

    @ remove space used to store spsr
    add     sp, sp, #4
    movs    pc, lr
