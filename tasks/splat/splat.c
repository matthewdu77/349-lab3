/** @file splat.c
 *
 * @brief Displays a spinning cursor.
 *
 * Links to libc.
 */

#include <stdlib.h>
#include <errno.h>
#include "bits/fileno.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
  char splat1[3] = "\b|";
  char splat2[3] = "\b/";
  char splat3[3] = "\b-";
  char splat4[3] = "\b\\";
  while (1)
  {
    write(STDOUT_FILENO, splat1, sizeof(splat1));
    sleep(200);
    write(STDOUT_FILENO, splat2, sizeof(splat2));
    sleep(200);
    write(STDOUT_FILENO, splat3, sizeof(splat3));
    sleep(200);
    write(STDOUT_FILENO, splat4, sizeof(splat3));
    sleep(200);
  }
  return 0;
}

