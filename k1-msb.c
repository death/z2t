/*
 * k1-msb - Compute the most significant byte of K1, given K2 and K2p.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned int k2;
    unsigned int k2p;
    unsigned char k1msb;

    setup();

    if (argc < 3) {
        fprintf(stderr, "usage: k1-msb <k2> <k2p>\n");
        return 1;
    }

    k2 = (unsigned int)strtoul(argv[1], 0, 16);
    k2p = (unsigned int)strtoul(argv[2], 0, 16);

    k1msb = (unsigned char)crc32i(k2, (unsigned char)k2p);

    printf("%02X\n", k1msb);

    return 0;
}
