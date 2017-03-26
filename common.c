/*
 * common - Operations common to Z2T programs.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

/*
 * Ensure that some of our assumptions are correct; exit otherwise.
 */
static void sanity_checks(void)
{
  if (sizeof(unsigned short) != 2) {
    fprintf(stderr, "Must have 16-bit unsigned shorts\n");
    exit(1);
  }

  if (sizeof(unsigned int) != 4) {
    fprintf(stderr, "Must have 32-bit unsigned ints\n");
    exit(1);
  }
}

/*
 * Do the setup needed for common operations.
 */
void setup(void)
{
  sanity_checks();
}

/*
 * Produce the 64 possible values of temp, given a particular k3
 * value.
 */
void k3_to_temp_list(unsigned char k3, unsigned short *temps)
{
  int i;
  int j;

  for (i = 0, j = 0; i < (1 << 14); i++) {
    unsigned short temp = ((i & 0xFFFF) << 2) | 3;
    unsigned char d = (temp * (temp ^ 1)) >> 8;
    if (d == k3) {
      temps[j] = temp;
      j++;
    }
  }
}
