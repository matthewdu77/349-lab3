/*
 * globals.h: Defines global variables for the program.
 *
 * Author: Harry Q Bovik <hqbovik@andrew.cmu.edu>
 * Date:   Tue, 23 Oct 2007 11:20:33 -0400
 */

#ifndef GLOBALS_H
#define GLOBALS_H

extern int user_setup_stack_ptr;
extern int global_data;
extern int timer_ret_address;
extern int irq_stack_space[1024];

#endif /* GLOBALS_H */
