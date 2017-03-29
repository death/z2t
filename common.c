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

/*
 * Do the setup needed for common operations.
 */
void setup(void)
{
    sanity_checks();
    crc32_init();
}

/*
 * Produce the 64 possible values of temp, given a particular k3
 * value.
 */
void k3_to_temp_list(unsigned char k3, unsigned short *temps)
{
    int i;
    int j;

    for (i = 0, j = 0; i < (1 << 14); i++) {
        unsigned short temp = ((i & 0xFFFF) << 2) | 3;
        unsigned char d = (temp * (temp ^ 1)) >> 8;
        if (d == k3) {
            temps[j] = temp;
            j++;
        }
    }
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
unsigned int crc32i(unsigned char nextcrc, unsigned char c)
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
    s->k1 = s->k1 + (s->k0 & 0xFF);
    s->k1 = s->k1 * 0x08088405 + 1;
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
