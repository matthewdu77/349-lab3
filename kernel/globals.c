/*
 * globals.c: Defines the global variables
 *
 * Author: Mike Kasick <mkasick@andrew.cmu.edu>
 * Date:   Sun, 07 Oct 2007 01:31:00 -0400
 */

int user_setup_stack_ptr;
int global_data;
int irq_stack_space[2049];

int timer_ret_address;
int timer_active;
unsigned long timer_end_time;
unsigned long system_time;
