/** @file typo.c
 *
 * @brief Echos characters back with timing data.
 *
 * Links to libc.
 */

#include <stdio.h>
#include <unistd.h>

#define BUF_LEN 256

int main(int argc, char** argv)
{
  char first;
  char rest[BUF_LEN+1];
  ssize_t num_chars;

  while (1)
  {
    // get first char, ignoring newlines and EOT characters
    do 
    {
      printf("> ");
      num_chars = read(STDIN_FILENO, &first, 1);
    } 
    while(num_chars == 0 || first == '\n' || first == 4);

    // after getting the first char, load the time
    unsigned long initial_time = time();

    // read the rest of the line, and null-terminate it
    num_chars = read(STDIN_FILENO, rest, BUF_LEN);
    rest[num_chars] = '\0';

    // get the elapsed time
    unsigned long elapsed_time = time() - initial_time;

    // writes out what we just read, along with the time in seconds
    putchar(first);
    printf("%s", rest);
    printf("%d.%03d\n", (int) elapsed_time / 1000, (int) elapsed_time % 1000);
  }
	return 0;
}
