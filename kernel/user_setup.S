@ user_setup.S: helper function written in asm called from kernel
@
@ Author: Anand Pattabiraman <apattabi@andrew.cmu.edu>
@ Date:   Wed, 29 Oct 2014
@
@ Hint:
@   CPSR:
@       N|Z|C|V| [27:9] |I|F|T|M[4:0]
@   I -> 1
@   F -> 1
@   T -> T
@   CPSR_SVC = 0x10011 -> CPSR_USER = 0x10000

    .extern user_setup_stack_ptr
    .extern irq_stack_space

    #include <arm/psr.h>
    #include <bits/swi.h>

    .extern print_int
    .file   "user_setup.S"
    .text


    .global user_setup
user_setup:
    @ Save gp registers, lr, and cpsr so that we can return to kernel main later.
    stmfd   sp!, {r1-r12, lr}
    mrs     r2, cpsr
    stmfd   sp!, {r2}

    @ Save the stack pointer in a global variable for exit_handler.S.
    ldr     r2, =user_setup_stack_ptr
    str     sp, [r2] @ *user_setup_stack_ptr = sp

    @ changes mode to IRQ
    mrs     r2, cpsr
    bic     r2, r2, #PSR_MODE
    orr     r2, r2, #PSR_MODE_IRQ  @ Change mode bits to irq
    msr     cpsr, r2

    @ sets up irq stack
    ldr     sp, =irq_stack_space
    ldr     r2, =0x00002000
    add     sp, sp, r2

    @ change mode to USER
    mrs     r2, cpsr
    orr     r2, r2, #PSR_FIQ  @ Disables fiqs
    bic     r2, r2, #PSR_IRQ  @ Enables irqs
    bic     r2, r2, #PSR_MODE
    orr     r2, r2, #PSR_MODE_USR  @ Change mode bits to usr
    msr     cpsr, r2

    @ set up user stack from spTop passed in from kernel.c
    mov     sp, r0

    @ go to user program
    ldr     pc, =0xa0000000

    @@@ exit syscall essentially returns through here!
