/*
 * k2-candidates - Print K2 candidates (30 most significant bits of
 * each, actually) given a sequence of K3 values (ordered from first
 * known to last known).
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int remove_duplicates(unsigned int *in, int inlen, unsigned int *out);

int main(int argc, char *argv[])
{
    static unsigned int candidates[1 << 22];
    static unsigned int temp[1 << 22];
    int n;
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: k2-candidates <k3_0> [k3_1] ...\n");
        return 1;
    }

    setup();

    for (i = argc - 1; i > 0; i--) {
        unsigned char k3 = (unsigned char)strtoul(argv[i], 0, 16);
        if (i == argc - 1) {
            k2_candidates_initial(k3, candidates);
            n = 1 << 22;
        } else {
            int m = k2_candidates_previous(k3, candidates, n, temp);
            n = remove_duplicates(temp, m, candidates);
        }
    }

    for (i = 0; i < n; i++) {
        printf("%08X\n", candidates[i]);
    }

    return 0;
}

static int compare(const void *a, const void *b);

int remove_duplicates(unsigned int *in, int inlen, unsigned int *out)
{
    int i;
    unsigned int *out0 = out;

    qsort(in, inlen, sizeof(unsigned int), compare);
    for (i = 0; i < inlen; i++) {
        if (i == 0 || in[i - 1] != in[i]) {
            *out++ = in[i];
        }
    }

    return out - out0;
}

int compare(const void *a, const void *b)
{
    unsigned int x = *(unsigned int *)a;
    unsigned int y = *(unsigned int *)b;
    if (x < y)
        return -1;
    else if (x > y)
        return 1;
    else
        return 0;
}
