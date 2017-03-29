/*
 * key-states - Print key states along the way of encrypting a stream
 * of octets.
 */
#include <stdio.h>

#include "common.h"

int main(int argc, char *argv[])
{
    struct keys s;
    int c = EOF;

    setup();

    keys_init(&s, argc < 2 ? "" : argv[1]);

    while (1) {
        unsigned char k3 = keys_k3(&s);
        if (c != EOF) {
            printf("c=%02X ", c);
        } else {
            printf("     ");
        }
        printf("k0=%08X k1=%08X k2=%08X (k3=%02X)\n", s.k0, s.k1, s.k2, k3);
        if ((c = fgetc(stdin)) == EOF) {
            break;
        }
        keys_update(&s, c);
    }

    return 0;
}
