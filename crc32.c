/*
 * crc32 - Print 32-bit cyclic redundancy checksum of octets from
 * standard input.
 */
#include <stdio.h>

#include "common.h"

int main(void)
{
    int c;
    unsigned int crc = 0xFFFFFFFF;

    setup();

    while ((c = fgetc(stdin)) != EOF) {
        crc = crc32(crc, (unsigned char)c);
    }

    crc = ~crc;

    printf("%08X\n", crc);

    return 0;
}
