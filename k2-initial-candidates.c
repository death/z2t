/*
 * k2-initial-candidates - Print all 2^22 initial candidates for k2[n]
 * given a k3[n] value.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned char k3;
    const unsigned short *temps;
    int i;

    setup();

    if (argc < 2) {
        fprintf(stderr, "usage: k2-initial-candidates <k3>\n");
        return 1;
    }

    k3 = (unsigned char)strtoul(argv[1], 0, 16);
    temps = temp_candidates(k3);

    for (i = 0; i < 64; i++) {
        unsigned short temp = temps[i];
        unsigned short k2msbs = 0;

        do {
            unsigned int k2 = k2msbs;
            k2 <<= 16;
            k2 |= temp;
            printf("%08X\n", k2);
            k2msbs++;
        } while (k2msbs != 0);
    }

    return 0;
}
