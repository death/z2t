/*
 * k0-lsbs - Compute the least significant bytes of K0 values, given a
 * sequence of K1 values.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned int k1[MAX_BYTES];
    unsigned char k0lsb[MAX_BYTES];
    int i;
    int len;

    setup();

    if (argc < 3) {
        fprintf(stderr, "usage: k0-lsbs <k1_0> <k1_1> ...\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        k1[len] = (unsigned int)strtoul(argv[i], 0, 16);
        len++;
        if (len == MAX_BYTES) {
            break;
        }
    }

    k0_lsbs(k1, len, k0lsb);

    for (i = 0; i < len - 1; i++) {
        printf("%02X%c", k0lsb[i], i == len - 2 ? '\n' : ' ');
    }

    return 0;
}
