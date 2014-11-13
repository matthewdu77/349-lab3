@ sleep_handler.S: function written in asm called from kernel
@
@ Author: Harry Q Bovik <hqbovik@andrew.cmu.edu>
@ Date:   Tue, 23 Oct 2007 11:20:33 -0400
@

    .extern enableTimerInterrupts
    .extern timer_ret_address

    .file   "sleep_handler.S"
    .text

    .global sleep_handler
sleep_handler:
    @ store sp and save lr
    stmfd sp!, {lr}
    ldr r1, =sp_store
    str sp, [r1]

    bl helper1

    @ restore sp and return
    ldr r1, =sp_store
    ldr sp, [r1]
    ldmfd sp!, {pc}
   
helper1:
    ldr r1, =timer_ret_address
    str lr, [r1]
    bl enableTimerInterrupts

sp_store:
    .word 0
