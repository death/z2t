/*
 * common - Operations common to Z2T programs.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * Parse octets encoded as hex values in the arguments vector until
 * there are no more arguments left or MAX_BYTES values have been
 * parsed.  Return the number of values parsed.
 *
 * The output buffer should have capacity of MAX_BYTES.
 */
int parse_hex_bytes(char **argv, unsigned char *out)
{
    int i;

    for (i = 0; argv[i] && i < MAX_BYTES; i++) {
        out[i] = (unsigned char)strtoul(argv[i], 0, 16);
    }

    return i;
}

/*
 * Reverse the bytes in the buffer.
 */
void reverse_bytes(unsigned char *bytes, int len)
{
    int i;

    for (i = 0; i < len / 2; i++) {
        unsigned char c = bytes[i];
        bytes[i] = bytes[len - i - 1];
        bytes[len - i - 1] = c;
    }
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
 * Compute possible partial candidates for the previous K2 value,
 * given a particular K2 value and the previous K3 value.
 *
 * The output buffer should have capacity for 64 elements.  The number
 * of candidates filled out is returned.
 */
int k2p_partial_candidates(unsigned int k2, unsigned char k3p, unsigned int *out)
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
 * Compute the 2^22 partial candidates for K2 given a K3 value.
 */
static void k2_partial_candidates_initial(unsigned char k3, unsigned int *out)
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
 * Compute the previous K2's partial candidates given a list of K2
 * candidates and a previous K3 value.  The new number of candidates
 * is returned.
 *
 * The output buffer should have capacity for 2^22 candidates.  It may
 * contain duplicates, which can later be removed.
 */
static int k2_partial_candidates_previous(unsigned char k3p, unsigned int *in, int inlen, unsigned int *out)
{
    int i;
    unsigned int *out0 = out;

    for (i = 0; i < inlen; i++) {
        unsigned int k2 = in[i];
        unsigned int k2p[64];
        int numk2p = k2p_partial_candidates(k2, k3p, k2p);
        int j;
        for (j = 0; j < numk2p; j++) {
            *out++ = k2p[j];
        }
    }

    return out - out0;
}

static int k2_remove_duplicates(unsigned int *in, int inlen, unsigned int *out);

/*
 * Compute K2 partial candidates (the 30 most significant bits for
 * each) for initial K3 value, given a list of K3 values.  Returns a
 * pointer to the array of candidates, and sets *outlen to its size.
 */
unsigned int *k2_partial_candidates(unsigned char *k3, int k3len, int *outlen)
{
    static unsigned int candidates[1 << 22];
    static unsigned int temp[1 << 22];
    int i;
    int n = 0;

    for (i = k3len - 1; i >= 0; i--) {
        if (i == k3len - 1) {
            k2_partial_candidates_initial(k3[i], candidates);
            n = 1 << 22;
        } else {
            int m = k2_partial_candidates_previous(k3[i], candidates, n, temp);
            n = k2_remove_duplicates(temp, m, candidates);
        }
    }

    *outlen = n;
    return candidates;
}

static int k2_compare(const void *a, const void *b);

/*
 * Remove duplicate K2 values.
 */
int k2_remove_duplicates(unsigned int *in, int inlen, unsigned int *out)
{
    int i;
    unsigned int *out0 = out;

    qsort(in, inlen, sizeof(unsigned int), k2_compare);
    for (i = 0; i < inlen; i++) {
        if (i == 0 || in[i - 1] != in[i]) {
            *out++ = in[i];
        }
    }

    return out - out0;
}

/*
 * Comparison function for sorting K2 values.
 */
int k2_compare(const void *a, const void *b)
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

/*
 * Given a sequence of K2 values, compute the most significant bytes
 * of K1 values for each adjacent pair (resulting in k2len-1 values).
 */
void k1_msbs(unsigned int *k2, int k2len, unsigned char *out)
{
    int i;

    for (i = 0; i < k2len - 1; i++) {
        unsigned int p = k2[i];
        unsigned int v = k2[i + 1];
        out[i] = (unsigned char)crc32i(v, (unsigned char)p);
    }
}

/*
 * Given a sequence of K1 values, compute the least significant bytes
 * of K0 values for each adjacent pair (resulting in k1len-1 values).
 */
void k0_lsbs(unsigned int *k1, int k1len, unsigned char *out)
{
    int i;

    for (i = 0; i < k1len - 1; i++) {
        unsigned int p = k1[i];
        unsigned int v = k1[i + 1];
        out[i] = (unsigned char)(lcgi(v) - p);
    }
}

/*
 * Given 4 least significant bytes of adjacent K0 values and the 4
 * plaintext bytes for those positions, recover the full K0 value for
 * the last position.
 */
unsigned int k0_recover4(unsigned char *b, unsigned char *p)
{
    unsigned int k0;

    k0 = crc32(b[0], p[0]);
    k0 >>= 8;
    k0 ^= crc32(b[1], p[1]);
    k0 >>= 8;
    k0 ^= crc32(b[2], p[2]);
    k0 >>= 8;
    k0 ^= crc32(b[3], p[3]);

    return k0;
}

/*
 * Dump a sequence of values to standard output.
 */
void list_dump(void *context, unsigned int *list, int len)
{
    int i;
    (void)context;
    for (i = 0; i < len; i++) {
        printf("%08X%c", list[i], i == len - 1 ? '\n' : ' ');
    }
}

static void k2_candidate_lists_aux(unsigned int *k2, unsigned char *k3, int i, int len, void *context, list_receiver receiver);

/*
 * Given a K2 partial candidate and a sequence of K3 values (capped at
 * MAX_BYTES), generate sequences containing complete K2 values
 * (except the first one, which will be partial).  Each sequence is a
 * candidate for being the true K2 sequence, and the list receiver
 * will be called with it.
 */
void k2_candidate_lists(unsigned int k2partial, unsigned char *k3, int k3len, void *context, list_receiver receiver)
{
    unsigned int k2[MAX_BYTES + 1];
    k2[k3len] = k2partial & 0xFFFFFFFC;
    k2_candidate_lists_aux(k2, k3, k3len, k3len + 1, context, receiver);
}

/*
 * Use k3[i-1] to compute a new partial candidate in k2[i-1] and
 * complete the partial candidate in k2[i].
 *
 * When there are no more candidates to complete, call the list
 * receiver with the list.
 */
void k2_candidate_lists_aux(unsigned int *k2, unsigned char *k3, int i, int len, void *context, list_receiver receiver)
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
            receiver(context, k2, len);
        } else {
            k2_candidate_lists_aux(k2, k3, i - 1, len, context, receiver);
        }
    }
}

static void k1_candidate_lists_aux(unsigned int *k1, unsigned char *msb, int i, int len, void *context, list_receiver receiver);

/*
 * Given a list (capped at MAX_BYTES) of the most significant bytes of
 * K1 values, call the list receiver for each K1 candidate list (there
 * will be around 2^16 of them).  The first value in each list has
 * only 23 known bits.
 *
 * This isn't 1994 - we don't need no lookup tables.
 */
void k1_candidate_lists(unsigned char *k1msbs, int k1len, void *context, list_receiver receiver)
{
    unsigned int i;
    unsigned int k1[MAX_BYTES];

    if (k1len < 2) {
        return;
    }

    for (i = 0; i < (1 << 24); i++) {
        k1[k1len - 1] = (k1msbs[k1len - 1] << 24) | i;
        if ((lcgi(k1[k1len - 1]) >> 24) != k1msbs[k1len - 2]) {
            continue;
        }

        k1_candidate_lists_aux(k1, k1msbs, 1, k1len, context, receiver);
    }
}

void k1_candidate_lists_aux(unsigned int *k1, unsigned char *msb, int i, int len, void *context, list_receiver receiver)
{
    unsigned int j;

    k1[len - i - 1] = lcgi(k1[len - i]);

    if (i + 1 == len) {
        /*
         * We don't know the first k1 value's least significant 9 bits.
         */
        k1[0] &= 0xFFFFFE00;
        receiver(context, k1, len);
        return;
    }

    for (j = 0; j < 256; j++) {
        if ((lcgi(k1[len - i - 1]) >> 24) == msb[len - i - 2]) {
            k1_candidate_lists_aux(k1, msb, i + 1, len, context, receiver);
        }
        k1[len - i - 1]--;
    }
}

/*
 * Given key state values and ciphertext bytes, go back to the initial
 * key state values.
 */
void keys_roll_back(struct keys *s, unsigned char *c, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        struct keys ps;
        unsigned short temp;
        unsigned char k3;
        unsigned char p;

        ps.k2 = crc32i(s->k2, s->k1 >> 24);
        ps.k1 = lcgi(s->k1) - (s->k0 & 0xFF);
        temp = (ps.k2 & 0xFFFF) | 3;
        k3 = (unsigned char)((temp * (temp ^ 1)) >> 8);
        p = c[len - i - 1] ^ k3;
        /* Biham's paper has a typo making it into crc32. */
        ps.k0 = crc32i(s->k0, p);

        memmove(s, &ps, sizeof(struct keys));
    }
}
