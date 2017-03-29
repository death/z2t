/*
 * k3-possible-temps - Print the possible values of temp given a
 * particular k3 value.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int main(int argc, char *argv[])
{
    unsigned char k3;
    unsigned short temps[64];
    int i;

    setup();

    if (argc < 2) {
        fprintf(stderr, "usage: k3-possible-temps <k3>\n");
        return 1;
    }

    k3 = (unsigned char)atoi(argv[1]);
    k3_to_temp_list(k3, temps);

    for (i = 0; i < 64; i++) {
        printf("%04X\n", temps[i]);
    }

    return 0;
}
