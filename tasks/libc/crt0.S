@ crt0.S: Execution startup routine
@
@ Authors: Anand Pattabiraman
@ Date:    October, 11 2014

#include <bits/swi.h>

	.file	"crt0.S"
	.text

	.global	_start
_start:
    ldr r0, [sp]   @ give argc
    add r1, sp, #4 @ give argv
    bl main
    swi EXIT_SWI
