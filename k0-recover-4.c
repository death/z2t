/*
 * k0-recover-4 - Given four K0 LSBs and corresponding plaintext
 * bytes, print recovered K0 values.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned char b[4];
    unsigned char p[4];
    unsigned int k0[4] = {0};
    int i;

    if (argc < 9) {
        fprintf(stderr, "usage: k0-recover-4 <b0> <p0> <b1> <p1> <b2> <p2> <b3> <p3>\n");
        return 1;
    }

    setup();

    for (i = 0; i < 4; i++) {
        b[i] = (unsigned char)strtoul(argv[(i * 2) + 1], 0, 16);
        p[i] = (unsigned char)strtoul(argv[(i * 2) + 2], 0, 16);
    }

    k0[3] = k0_recover4(b, p);

    for (i = 2; i >= 0; i--) {
        k0[i] = crc32i(k0[i + 1], p[i + 1]);
    }

    for (i = 0; i < 4; i++) {
        printf("%08X\n", k0[i]);
    }

    return 0;
}
