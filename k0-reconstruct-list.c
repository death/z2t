/*
 * k0-reconstruct-list - Given one full K0, a direction (forward or
 * backward), and plaintext bytes, calculate and print respective K0s.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned int k0;
    int forward;
    int i;

    if (argc < 3) {
        fprintf(stderr, "usage: k0-reconstruct-list <k0> <direction> [p0] [p1] ... [pn]\n");
        return 1;
    }

    setup();

    k0 = (unsigned int)strtoul(argv[1], 0, 16);
    forward = tolower(*argv[2]) == 'f';

    for (i = 3; i < argc; i++) {
        unsigned char p = (unsigned char)strtoul(argv[i], 0, 16);
        k0 = forward ? crc32(k0, p) : crc32i(k0, p);
        printf("%08X\n", k0);
    }

    return 0;
}
