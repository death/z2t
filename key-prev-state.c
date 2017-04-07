/*
 * key-prev-state - Given key state values and ciphertext bytes, go
 * back to the initial key state values.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

int main(int argc, char *argv[])
{
    struct keys s;
    int i;

    if (argc < 4) {
        fprintf(stderr, "usage: key-prev-state <k0> <k1> <k2> [c0] ...\n");
        return 1;
    }

    setup();

    s.k0 = (unsigned int)strtoul(argv[1], 0, 16);
    s.k1 = (unsigned int)strtoul(argv[2], 0, 16);
    s.k2 = (unsigned int)strtoul(argv[3], 0, 16);

    for (i = 4; i < argc; i++) {
        unsigned char c = (unsigned char)strtoul(argv[i], 0, 16);
        struct keys ps;
        unsigned short temp;
        unsigned char k3;
        unsigned char p;

        ps.k2 = crc32i(s.k2, s.k1 >> 24);
        ps.k1 = lcgi(s.k1) - (s.k0 & 0xFF);
        temp = (ps.k2 & 0xFFFF) | 3;
        k3 = (unsigned char)((temp * (temp ^ 1)) >> 8);
        p = c ^ k3;
        /* Biham's paper has a typo making it into crc32. */
        ps.k0 = crc32i(s.k0, p);

        memmove(&s, &ps, sizeof(struct keys));
    }

    printf("k0=%08X k1=%08X k2=%08X\n", s.k0, s.k1, s.k2);

    return 0;
}
