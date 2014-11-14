@ swi_handler.S: SWI Handler
@
@ Author: Harry Q Bovik <hqbovik@andrew.cmu.edu>
@ Date:   Tue, 23 Oct 2007 11:20:33 -0400
@

    .extern C_IRQ_Handler

    .file   "irq_handler.S"
    .text

    .global irq_handler
irq_handler:
    adds pc, lr, #4

    sub     sp, sp, #4
    stmfd   sp!, {r0-r12}
    stmfd   sp!, {lr}

    @ get spsr into general purpose (gp) registers
    mrs     r2, spsr

    @ store spsr above gp registers
    str     r2, [sp, #14*4]

    @ restores r8 function table
    ldr     r8, =global_data
    ldr     r8, [r8]

    @ go to handler
    bl      C_IRQ_Handler

    @ set return address appropriately depending on the return value of
    @ the handler. (If r0 != -1), then lr = r0. Otherwise, lr = saved value
    /*
    add     r0, r0, #1
    subne   lr, r0, #1
    */

    @ restore spsr
    ldr     r2, [sp, #14*4]
    msr     spsr, r2

    @ restore registers.
    ldmfd   sp!, {lr}
    ldmfd   sp!, {r0-r12}

    @ remove space used to store spsr
    add     sp, sp, #4
    movs    pc, lr

