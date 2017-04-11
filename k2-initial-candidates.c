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
    int i;
    static unsigned int k2[1 << 22];

    setup();

    if (argc < 2) {
        fprintf(stderr, "usage: k2-initial-candidates <k3>\n");
        return 1;
    }

    k3 = (unsigned char)strtoul(argv[1], 0, 16);
    k2_candidates_initial(k3, k2);

    for (i = 0; i < (1 << 22); i++) {
        printf("%08X\n", k2[i]);
    }

    return 0;
}
