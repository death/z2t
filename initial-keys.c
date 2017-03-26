/*
 * initial-keys - Print initial keys for a given password.
 */
#include <stdio.h>

#include "common.h"

int main(int argc, char *argv[])
{
  struct keys s;

  setup();

  if (argc < 2) {
    fprintf(stderr, "usage: initial-keys <password>\n");
    return 1;
  }

  keys_init(&s, argv[1]);

  printf("k0=%08X k1=%08X k2=%08X (k3=%02X)\n", s.k0, s.k1, s.k2, keys_k3(&s));

  return 0;
}
