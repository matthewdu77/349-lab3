#include <exports.h>
#include <bits/errno.h>
#include <bits/fileno.h>
#include <bits/swi.h>

#include <arm/psr.h>
#include <arm/exception.h>
#include <arm/interrupt.h>
#include <arm/timer.h>
#include <arm/reg.h>

#include "globals.h"
#include "swi_handler.h"
#include "irq_handler.h"
#include "user_setup.h"
#include "exit_handler.h"
#include "sleep_handler.h"

typedef enum {false, true} bool;

// 0xe59ff014 (LDR pc, [pc, 0x14]) --> 0x014 through masking
#define SWI_VECT_ADDR 0x08
#define PC_OFFSET 0x08
// Cannot write to this address. kernel.bin loaded here. Stack grows down.
#define USER_STACK_TOP 0xa3000000

// (LDR pc, [pc, 0x000]) 0xe59ff000 --> 0x000 through masking
#define LDR_PC_PC_INSTR 0xe59ff000
#define LDR_SIGN_MASK 0x00800000

#define BAD_CODE 0x0badc0de

#define SFROM_START 0x00000000
#define SFROM_END 0x00ffffff
#define SDRAM_START 0xa0000000
#define SDRAM_END 0xa3ffffff

/* Checks the Vector Table. */
bool check_vector(int exception_num)
{
  int vector_instr = *((int *)(exception_num * 4));

  // Check if the offset is negative.
  if ((vector_instr & LDR_SIGN_MASK) == 0)
  {
    return false;
  }

  // Check that the instruction is a (LDR pc, [pc, 0x000])
  if ((vector_instr & 0xFFFFF000) != LDR_PC_PC_INSTR)
  {
    return false;
  }

  return true;
}

void install_exception_handler(int exception_num, int new_handler, int* old_instructions)
{
  int vector_address = exception_num * 4;

  // Jump offset already incorporates PC offset. Usually 0x10 or 0x14.
  int jmp_offset = (*((int *) vector_address))&(0xFFF);

  // &Handler" in Jump Table.
  int *handler_addr = *(int **)(vector_address + PC_OFFSET + jmp_offset);

  // Save original Uboot handler instructions.
  old_instructions[0] = *handler_addr;
  old_instructions[1] = *(handler_addr + 1);

  // Wire in our own: LDR pc, [pc, #-4] = 0xe51ff004
  *handler_addr = 0xe51ff004;
  *(handler_addr + 1) = new_handler;
}

void restore_exception_handler(int exception_num, int* old_instructions)
{
  int vector_address = exception_num * 4;

  // Jump offset already incorporates PC offset. Usually 0x10 or 0x14.
  int jmp_offset = (*((int *) vector_address))&(0xFFF);

  // &Handler" in Jump Table.
  int *handler_addr = *(int **)(vector_address + PC_OFFSET + jmp_offset);

  // Restore original Uboot handler instructions.
  *handler_addr = old_instructions[0];
  *(handler_addr + 1) = old_instructions[1];
}

int kmain(int argc, char** argv, uint32_t table)
{
  app_startup(); /* Note that app_startup() sets all uninitialized and */ 
  /* zero global variables to zero. Make sure to consider */
  /* any implications on code executed before this. */
  global_data = table;

  // Installs the swi handler
  if (check_vector(EX_SWI) == false)
  {
    return BAD_CODE;
  }
  int swi_instrs[2] = {0};
  install_exception_handler(EX_SWI, (int) &swi_handler, swi_instrs);
  
  // Installs the irq handler
  if (check_vector(EX_IRQ) == false)
  {
    return BAD_CODE;
  }
  int irq_instrs[2] = {0};
  install_exception_handler(EX_SWI, (int) &irq_handler, irq_instrs);

  // Copy argc and argv to user stack in the right order.
  int *spTop = ((int *) USER_STACK_TOP) - 1;
  int i = 0;
  for (i = argc-1; i >= 0; i--)
  {
    *spTop = (int)argv[i];
    spTop--;
  }
  *spTop = argc;

  /** Jump to user program. **/
  int usr_prog_status = user_setup(spTop);

  /** Restore SWI and IRQ Handlers. **/
  restore_exception_handler(EX_SWI, swi_instrs);
  restore_exception_handler(EX_IRQ, irq_instrs);

  return usr_prog_status;
}

/* Verifies that the buffer is entirely in valid memory. */
int check_mem(char *buf, int count, unsigned start, unsigned end) 
{
  unsigned start_buf = (unsigned) buf;
  unsigned end_buf = (unsigned)(buf + count);

  if ((start_buf < start) || (start_buf > end)) 
  {
    return false;
  }
  if ((end_buf < start) || (end_buf > end)) 
  {
    return false;
  }
  // Overflow case.
  if (start_buf > end_buf)
  {
    return false;
  }

  return true;
}

// write function to replace the system's write function
ssize_t write_handler(int fd, const void *buf, size_t count)
{
  // Check for invalid memory range or file descriptors
  if (check_mem((char *) buf, (int) count, SDRAM_START, SDRAM_END) == false &&
      check_mem((char *) buf, (int) count, SFROM_START, SFROM_END) == false)
  {
    exit_handler(-EFAULT);
  }
  else if (fd != STDOUT_FILENO)
  {
    exit_handler(-EBADF);
  }

  char *buffer = (char *) buf;
  size_t i;
  char read_char;
  for (i = 0; i < count; i++)
  {
    // put character into buffer and putc
    read_char = buffer[i];
    putc(read_char);
  }
  return i;
}

// read function to replace the system's read function
ssize_t read_handler(int fd, void *buf, size_t count)
{
  // Check for invalid memory range or file descriptors
  if (check_mem((char *) buf, (int) count, SDRAM_START, SDRAM_END) == false)
  {
    exit_handler(-EFAULT);
  }
  else if (fd != STDIN_FILENO)
  {
    exit_handler(-EBADF);
  }

  size_t i = 0;
  char *buffer = (char *) buf;
  char read_char;

  while (i < count)
  {
    read_char = getc();

    if (read_char == 4)
    { //EOT character
      return i;
    }
    else if (((read_char == 8) || (read_char == 127)))
    { // backspace or DEL character
      buffer[i] = 0; // '\0' character
      if(i > 0)
      {
        i--;
        puts("\b \b");
      }
    }
    else if ((read_char == 10) || (read_char == 13))
    { // '\n' newline or '\r' carriage return character
      buffer[i] = '\n';
      putc('\n');
      return (i+1);
    }
    else
    {
      // put character into buffer and putc
      buffer[i] = read_char;
      i++;
      putc(read_char);
    }
  }

  return i;
}

unsigned long time_handler()
{
  return reg_read(OSTMR_OSCR_ADDR) / (OSTMR_FREQ/1000);
}

void enableTimerInterrupts(unsigned long millis)
{
  // activates interupts on this match register
  reg_set(OSTMR_OIER_ADDR, OSTMR_OIER_E0);

  // sets the 1st timer match register to the proper value
  int end_time = reg_read(OSTMR_OSCR_ADDR) + millis * (OSTMR_FREQ/1000);
  reg_write(OSTMR_OSMR_ADDR(0), end_time);

  // in a real system, we'd invoke the scheduler to run some other process,
  // but here we just loop infinitely
  while (1);
}

/* C_SWI_Handler uses SWI number to call the appropriate function. */
int C_SWI_Handler(int swiNum, int *regs) 
{
  int count = 0;
  switch (swiNum)
  {
    // ssize_t read(int fd, void *buf, size_t count);
    case READ_SWI:
      count = read_handler(regs[0], (void *) regs[1], (size_t) regs[2]);
      break;
      // ssize_t write(int fd, const void *buf, size_t count);
    case WRITE_SWI:
      count = write_handler((int) regs[0], (void *) regs[1], (size_t) regs[2]);
      break;
      // void exit(int status);
    case EXIT_SWI:
      exit_handler((int) regs[0]); // never returns
      break;
    case TIME_SWI:
      // unsigned long time();
      count = time_handler();
      break;
    case SLEEP_SWI:
      // void sleep(unsigned long millis);
      sleep_handler((unsigned long) regs[0]);
      break;
    default:
      printf("Error in ref C_SWI_Handler: Invalid SWI number.");
      exit_handler(BAD_CODE); // never returns
  }
  return count;
}
