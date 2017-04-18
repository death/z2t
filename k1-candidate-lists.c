/*
 * k1-candidate-lists - Given a list of the most significant bytes of
 * K1 values, print all possible K1 candidate lists (around 2^16 of
 * them).  The last value in each list has only 23 known bits.
 */
#include <stdio.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned char msb[MAX_BYTES];
    int len;

    setup();

    if (argc < 3) {
        fprintf(stderr, "usage: k1-candidate-lists <k1msb> <k1pmsb> [k1ppmsb] ...\n");
        return 1;
    }

    len = parse_hex_bytes(&argv[1], msb);
    k1_candidate_lists(msb, len, 0, list_dump);

    return 0;
}
