/*
 * k2p-candidates - Print the possible previous K2 values (30 most
 * significant bits of each value) given a particular K2 value and the
 * previous K3 value.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned int k2;
    unsigned char k3p;
    unsigned int numk2p;
    unsigned int k2p[64];
    int i;

    setup();

    if (argc < 3) {
        fprintf(stderr, "usage: k2p-candidates <k2> <k3p>\n");
        return 1;
    }

    k2 = (unsigned int)strtoul(argv[1], 0, 16);
    k3p = (unsigned char)strtoul(argv[2], 0, 16);

    numk2p = k2p_candidates(k2, k3p, k2p);

    for (i = 0; i < numk2p; i++) {
        printf("%08X\n", k2p[i]);
    }

    return 0;
}
