/*
 * k1-msbs - Compute the most significant bytes of K1 values, given a
 * sequence of K2 values.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned int k2[MAX_BYTES];
    unsigned char k1msb[MAX_BYTES];
    int i;
    int len = 0;

    setup();

    if (argc < 3) {
        fprintf(stderr, "usage: k1-msbs <k2_0> <k2_1> ...\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        k2[len] = (unsigned int)strtoul(argv[i], 0, 16);
        len++;
        if (len == MAX_BYTES) {
            break;
        }
    }

    k1_msbs(k2, len, k1msb);

    for (i = 0; i < len - 1; i++) {
        printf("%02X%c", k1msb[i], i == len - 2 ? '\n' : ' ');
    }

    return 0;
}
