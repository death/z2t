/*
 * k1-08088405-inv - Compute the inverse of 0x08088405 (mod 2^32).
 *
 * Spoiler: it's 0xD94FA8CD.
 *
 * First, I ran this with CHECK_UPPER of 256; this found the magic
 * value.  Then I verified that this works for all values by setting
 * INV_LOWER to it, INV_UPPER the its successor, and CHECK_UPPER to 0.
 */
#include <stdio.h>

#include "common.h"

enum {
    INV_LOWER = 0,
    INV_UPPER = 0,
    CHECK_LOWER = 0,
    CHECK_UPPER = 1 << 8
};

int main(void)
{
    unsigned int inv = INV_LOWER;
    do {
        unsigned int good = 1;
        unsigned int i = CHECK_LOWER;
        do {
            unsigned int f = i * 0x08088405 + 1;
            unsigned int b = (f - 1) * inv;
            if (b != i) {
                good = 0;
                break;
            }
            i++;
        } while (i != CHECK_UPPER);
        if (good) {
            printf("%08X\n", inv);
        }
        inv++;
    } while (inv != INV_UPPER);

    return 0;
}
