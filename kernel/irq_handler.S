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

    stmfd   sp!, {r0-r12, lr}

    @ restores r8 function table
    ldr     r8, =global_data
    ldr     r8, [r8]

    @ go to handler
    bl      C_IRQ_Handler

    @ restore registers and return
    ldmfd   sp!, {r0-r12, lr}
    subs    pc, lr, #4

