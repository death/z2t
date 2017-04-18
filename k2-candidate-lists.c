/*
 * k2-candidate-lists - Given a sequence of K3 values and K2 partial
 * candidates from standard input, print for each partial candidate a
 * list of an initial K2 partial candidate followed by complete
 * (32-bit) K2 values (the last one being the completed partial
 * candidate from the input).
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void dump(void *context, unsigned int *k2, int len);

int main(int argc, char *argv[])
{
    char line[64];
    unsigned char k3[MAX_BYTES];
    int numk3;

    if (argc < 2) {
        fprintf(stderr, "usage: k2-candidate-lists <k3_0> [k3_1] ...\n");
        return 1;
    }

    setup();

    numk3 = parse_hex_bytes(&argv[1], k3);

    while (fgets(line, sizeof(line), stdin)) {
        unsigned int k2partial = (unsigned int)strtoul(line, 0, 16);
        k2_candidate_lists(k2partial, k3, numk3, 0, list_dump);
    }

    return 0;
}
