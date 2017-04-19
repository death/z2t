/*
 * key-prev-state - Given key state values and ciphertext bytes
 * (supplied in reverse), go back to the initial key state values.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

int main(int argc, char *argv[])
{
    struct keys s;
    unsigned char c[MAX_BYTES];
    int numc;

    if (argc < 4) {
        fprintf(stderr, "usage: key-prev-state <k0> <k1> <k2> [c0] ...\n");
        return 1;
    }

    setup();

    s.k0 = (unsigned int)strtoul(argv[1], 0, 16);
    s.k1 = (unsigned int)strtoul(argv[2], 0, 16);
    s.k2 = (unsigned int)strtoul(argv[3], 0, 16);

    numc = parse_hex_bytes(&argv[4], c);
    reverse_bytes(c, numc);

    keys_roll_back(&s, c, numc);

    printf("k0=%08X k1=%08X k2=%08X\n", s.k0, s.k1, s.k2);

    return 0;
}
