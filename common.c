/*
 * common - Operations common to Z2T programs.
 */
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

/*
 * Ensure that some of our assumptions are correct; exit otherwise.
 */
static void sanity_checks(void)
{
    if (sizeof(unsigned short) != 2) {
        fprintf(stderr, "Must have 16-bit unsigned shorts\n");
        exit(1);
    }

    if (sizeof(unsigned int) != 4) {
        fprintf(stderr, "Must have 32-bit unsigned ints\n");
        exit(1);
    }
}

static void crc32_init(void);
static void temp_candidates_init(void);

/*
 * Do the setup needed for common operations.
 */
void setup(void)
{
    sanity_checks();
    crc32_init();
    temp_candidates_init();
}

/*
 * Temp candidates lookup tables.
 */
static unsigned short temp_candidates_table[256][64];

/*
 * Precompute temp candidates for each possible K3 value.
 */
void temp_candidates_init(void)
{
    unsigned char k3 = 0;

    do {
        int i;
        int j;

        for (i = 0, j = 0; i < (1 << 14); i++) {
            unsigned short temp = ((i & 0xFFFF) << 2) | 3;
            unsigned char d = (temp * (temp ^ 1)) >> 8;
            if (d == k3) {
                temp_candidates_table[k3][j] = temp;
                j++;
            }
        }

        k3++;
    } while (k3 != 0);
}

/*
 * Return the 64 possible temp values for a particular K3 value.
 */
const unsigned short *temp_candidates(unsigned char k3)
{
    return temp_candidates_table[k3];
}

/*
 * Return the successor of k1 in the series.
 */
unsigned int lcg(unsigned int k1)
{
    return k1 * 0x08088405 + 1;
}

/*
 * Return the predecessor of k1 in the series.
 */
unsigned int lcgi(unsigned int k1)
{
    return (k1 - 1) * 0xD94FA8CD;
}

/*
 * CRC32 lookup tables.
 */
static unsigned int crc32_table[256];
static unsigned int crc32_inv_table[256];

/*
 * CRC32 computation without lookup tables.
 */
static unsigned int crc32c(unsigned int prevcrc, unsigned char c)
{
    int j;
    unsigned crc = prevcrc;

    crc ^= c;
    for (j = 0; j < 8; j++) {
        if (crc & 1) {
            /*
             * The appnote refers to 0xDEBB20E3 (section 4.4.7); Biham's
             * paper gives the right value.
             */
            crc = (crc >> 1) ^ 0xEDB88320;
        } else {
            crc = crc >> 1;
        }
    }

    return crc;
}

/*
 * Precompute CRC32 lookup tables.
 */
void crc32_init(void)
{
    int i;

    for (i = 0; i < 256; i++) {
        unsigned int j = crc32c(0, i);
        crc32_table[i] = j;
        crc32_inv_table[j >> 24] = (j << 8) ^ i;
    }
}

/*
 * Compute the new CRC32 value given the previous value and an octet.
 */
unsigned int crc32(unsigned int prevcrc, unsigned char c)
{
    return (prevcrc >> 8) ^ crc32_table[(prevcrc & 0xFF) ^ c];
}

/*
 * Compute the CRC32 value before it was updated with the octet.
 */
unsigned int crc32i(unsigned int nextcrc, unsigned char c)
{
    return (nextcrc << 8) ^ crc32_inv_table[(nextcrc >> 24)] ^ c;
}

/*
 * Initialize keys with a password.
 *
 * Note that you can pass an empty string to get the hardcoded initial
 * values.
 */
void keys_init(struct keys *s, const char *pass)
{
    s->k0 = 0x12345678;
    s->k1 = 0x23456789;
    s->k2 = 0x34567890;

    while (*pass) {
        keys_update(s, *pass);
        pass++;
    }
}

/*
 * Get current keystream octet given keys state.
 */
unsigned char keys_k3(struct keys *s)
{
    /* We follow Biham in IORing with 3 rather than 2. */
    unsigned short temp = (s->k2 & 0xFFFF) | 3;
    return (temp * (temp ^ 1)) >> 8;
}

/*
 * Update keys state given an octet.
 */
void keys_update(struct keys *s, unsigned char c)
{
    s->k0 = crc32(s->k0, c);
    s->k1 = lcg(s->k1 + (s->k0 & 0xFF));
    s->k2 = crc32(s->k2, s->k1 >> 24);
}

/*
 * Crypt a buffer, updating the keys state on the way.
 */
void keys_crypt(struct keys *s, unsigned char *out, unsigned char *in, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        out[i] = in[i] ^ keys_k3(s);
        keys_update(s, out[i]);
    }
}

/*
 * Compute possible candidates for the previous K2 value (the 30 most
 * significant bits of it, actually), given a particular K2 value and
 * the previous K3 value.
 *
 * The output buffer should have capacity for 64 elements.  The number
 * of candidates filled out is returned.
 */
int k2p_candidates(unsigned int k2, unsigned char k3p, unsigned int *out)
{
    unsigned int rhs = crc32i(k2 & 0xFFFFFFFC, 0) & 0xFFFFFC00;
    const unsigned short *temps = temp_candidates(k3p);
    int i;
    int j;

    for (i = 0, j = 0; i < 64; i++) {
        unsigned int lhs = temps[i];

        if ((rhs & 0x0000FC00) == (lhs & 0x0000FC00)) {
            out[j] = (rhs | lhs) & 0xFFFFFFFC;
            j++;
        }
    }

    return j;
}

/*
 * Compute the 2^22 candidates for K2 given a K3 value (the 30 most
 * significant bits of each, actually).
 */
void k2_candidates_initial(unsigned char k3, unsigned int *out)
{
    const unsigned short *temps = temp_candidates(k3);
    int i;

    for (i = 0; i < 64; i++) {
        unsigned short temp = temps[i];
        unsigned short k2msbs = 0;

        do {
            unsigned int k2 = k2msbs;
            k2 <<= 16;
            k2 |= temp;
            *out++ = k2;
            k2msbs++;
        } while (k2msbs != 0);
    }
}

/*
 * Compute the previous K2 value candidates given a list of K2
 * candidates and a previous K3 value.  The new number of candidates
 * is returned.
 *
 * The output buffer should have capacity for 2^22 candidates.  It may
 * contain duplicates, which can later be removed.
 */
int k2_candidates_previous(unsigned char k3p, unsigned int *in, int inlen, unsigned int *out)
{
    int i;
    unsigned int *out0 = out;

    for (i = 0; i < inlen; i++) {
        unsigned int k2 = in[i];
        unsigned int k2p[64];
        int numk2p = k2p_candidates(k2, k3p, k2p);
        int j;
        for (j = 0; j < numk2p; j++) {
            *out++ = k2p[j];
        }
    }

    return out - out0;
}
