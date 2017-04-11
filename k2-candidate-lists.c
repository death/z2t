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

void follow(unsigned int *k2, unsigned char *k3, int i, int len);
void dump(unsigned int *k2, int len);

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
        unsigned int k2[MAX_BYTES + 1];
        k2[numk3] = (unsigned int)strtoul(line, 0, 16);
        k2[numk3] &= 0xFFFFFFFC;
        follow(k2, k3, numk3, numk3 + 1);
    }

    return 0;
}

/*
 * Use k3[i-1] to compute a new partial candidate in k2[i-1] and
 * complete the partial candidate in k2[i].
 *
 * When there are no more candidates to complete, print the K2 list.
 */
void follow(unsigned int *k2, unsigned char *k3, int i, int len)
{
    unsigned int k2save = k2[i];
    unsigned int k2p[64];
    int numk2p = k2p_partial_candidates(k2[i], k3[i - 1], k2p);
    int j;
    for (j = 0; j < numk2p; j++) {
        k2[i] = k2save;
        k2[i - 1] = k2p[j];

        /*
         * It may be possible to use a simpler computation here, but
         * who cares?
         */
        int lsbs;
        for (lsbs = 0; lsbs < 4; lsbs++) {
            unsigned int k2pcomplete = k2[i - 1] | lsbs;
            unsigned char k1msb = (unsigned char)crc32i(k2[i], (unsigned char)k2pcomplete);
            unsigned int k2complete = crc32(k2pcomplete, k1msb);
            if ((k2complete & 0xFFFFFFFC) == k2[i]) {
                k2[i] = k2complete;
                break;
            }
        }

        if (i == 1) {
            dump(k2, len);
        } else {
            follow(k2, k3, i - 1, len);
        }
    }
}

/*
 * Print K2 values to standard output.
 */
void dump(unsigned int *k2, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%08X%c", k2[i], i == len - 1 ? '\n' : ' ');
    }
}
