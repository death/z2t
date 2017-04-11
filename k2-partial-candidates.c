/*
 * k2-partial-candidates - Print K2 partial candidates (30 most
 * significant bits of each) given a sequence of K3 values (ordered
 * from first known to last known).
 */
#include <stdio.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned char k3[MAX_BYTES];
    unsigned int *k2;
    int numk3;
    int numk2;
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: k2-partial-candidates <k3_0> [k3_1] ...\n");
        return 1;
    }

    setup();

    numk3 = parse_hex_bytes(&argv[1], k3);
    k2 = k2_partial_candidates(k3, numk3, &numk2);

    for (i = 0; i < numk2; i++) {
        printf("%08X\n", k2[i]);
    }

    return 0;
}
