/*
 * k2-prev-candidates - Print all candidates for k2[n-1] given a list
 * of k2[n] candidates (from standard input) and k3[n-1].
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned char k3p;
    char line[64];

    setup();

    if (argc < 2) {
        fprintf(stderr, "usage: k2-prev-candidates <k3p>\n");
        return 1;
    }

    k3p = (unsigned char)strtoul(argv[1], 0, 16);

    while (fgets(line, sizeof(line), stdin)) {
        unsigned int k2 = (unsigned int)strtoul(line, 0, 16);
        unsigned int k2p[64];
        int numk2p = k2p_candidates(k2, k3p, k2p);
        int i;
        for (i = 0; i < numk2p; i++) {
            printf("%08X\n", k2p[i]);
        }
    }

    return 0;
}
