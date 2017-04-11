/*
 * k1-candidate-lists - Given a list of the most significant bytes of
 * K1 values, print all possible K1 candidate lists (around 2^16 of
 * them).  The last value in each list has only 23 known bits (and
 * only 20 of them are printed).
 *
 * This isn't 1994 - we don't need no lookup tables.
 */
#include <stdio.h>

#include "common.h"

void follow(unsigned int *k1, unsigned char *msb, int i, int len);

int main(int argc, char *argv[])
{
    unsigned char msb[MAX_BYTES];
    unsigned int k1[MAX_BYTES];
    unsigned int i;
    int len;

    setup();

    if (argc < 3) {
        fprintf(stderr, "usage: k1-candidate-lists <k1msb> <k1pmsb> [k1ppmsb] ...\n");
        return 1;
    }

    len = parse_hex_bytes(&argv[1], msb);

    for (i = 0; i < (1 << 24); i++) {
        k1[0] = (msb[0] << 24) | i;
        if ((lcgi(k1[0]) >> 24) != msb[1]) {
            continue;
        }

        follow(k1, msb, 1, len);
    }

    return 0;
}

void follow(unsigned int *k1, unsigned char *msb, int i, int len)
{
    unsigned int j;

    k1[i] = lcgi(k1[i - 1]);

    if (i + 1 == len) {
        for (j = 0; j < len - 1; j++) {
            printf("%08X ", k1[j]);
        }
        printf("%05X???\n", (k1[len - 1] & 0xFFFFF000) >> 12);
        return;
    }

    for (j = 0; j < 256; j++) {
        if ((lcgi(k1[i]) >> 24) == msb[i + 1]) {
            follow(k1, msb, i + 1, len);
        }
        k1[i]--;
    }
}
