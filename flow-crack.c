#define EXAMPLE
/*
 * flow-crack - Deduce key state from known plaintext and ciphertext
 * bytes.
 *
 * EXAMPLE
 *
 * We start by generating the message:
 *
 *  $ ./flow-gen-message > message.pgm
 *
 * Zip it without a password to get plaintext:
 *
 *  $ zip nopass.zip message.pgm
 *
 * Extract values of interest:
 *
 *  $ ./extract nopass.zip
 *
 * This gives:
 *
 *  (name "message.pgm"
 *   crc #x255BDEE0
 *   csize 17093
 *   usize 307215
 *   initial #(#xED #x9D #x2B #x90 #xE4 #xBA #xB6 #xA6
 *             #x85 #x0E #x38 #x71 #xD0 #x41 #x8D #x0A
 *             #x15 #x4A #x94 #x31 #x71 #xAF #x63 #x80
 *             #x91 #x91 #x91 #x63 #x80 #x91 #x91 #x90))
 *
 * Zip with a password to get ciphertext:
 *
 *  $ zip -e -P secret pass.zip message.pgm
 *
 * Extract values of interest:
 *
 *  $ ./extract pass.zip
 *
 * This gives:
 *
 *  (name "message.pgm"
 *   crc #x255BDEE0
 *   csize 17093
 *   usize 307215
 *   enc #(#xAB #x48 #x72 #x45 #x53 #xC5 #x43 #x2A
 *         #x13 #xBC #xBB #xBB)
 *   initial #(#xEE #x8B #xFE #x50 #xAF #x78 #xB3 #x4A
 *             #xB6 #xB1 #xA4 #xA5 #x2A #x7C #x51 #x9F
 *             #xA0 #x1E #x7D #x67 #x46 #xE7 #xCB #xCB
 *             #x17 #x89 #x69 #xC6 #xAC #x89 #xE3 #x02))
 *
 * Run flow crack (with EXAMPLE macro defined):
 *
 *  $ ./flow-crack -p ED9D2B90E4BAB6A6850E3871D0418D0A154A943171AF63809191916380919190 \
 *                 -c EE8BFE50AF78B34AB6B1A4A52A7C519FA01E7D6746E7CBCB178969C6AC89E302
 *
 * If we didn't have the EXAMPLE macro defined, it would search
 * through 388371 K2 candidate lists for this particular case.  We
 * could run multiple searches in parallel by supplying start/end
 * values.  Another good idea in that case is to use the `tee' utility
 * to log the output to a file.
 *
 * Since we do, in fact, have the EXAMPLE macro defined, most of the
 * false candidates are removed from the actual search, and it
 * finishes rather quickly, with the following result:
 *
 *  k0=5B7AE1A4 k1=789B988D k2=685D372D
 *
 * To get at the internal representation of the key, we need to also
 * consider the 12 prefix bytes:
 *
 *  $ ./key-prev-state 5B7AE1A4 789B988D 685D372D BB BB BC 13 2A 43 C5 53 45 72 48 AB
 *
 * This gives:
 *
 *  k0=DDA9E469 k1=96212999 k2=F9FC9651
 *
 * Now, recover the message using a crack-friendly unzip:
 *
 *  $ unzip -P DDA9E469-96212999-F9FC9651 pass.zip
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>

#include "common.h"

struct flow_context
{
    unsigned char *p;
    unsigned char *c;

    unsigned int *k2;
    int k2len;

    unsigned int *k1;
    int k1len;
};

void got_k2_candidate_list(void *context, unsigned int *k2, int len);
void got_k1_candidate_list(void *context, unsigned int *k1, int len);

int parse_hex_string(const char *s, unsigned char *out);
int crack(unsigned char *plaintext, unsigned char *ciphertext, int len, int k2start, int k2end);

int main(int argc, char *argv[])
{
    int k2start = -1;
    int k2end = -1;
    unsigned char plaintext[MAX_BYTES];
    unsigned char ciphertext[MAX_BYTES];
    int plaintextlen = -1;
    int ciphertextlen = -1;

    struct option longopts[] = {
        {"start"     , required_argument, 0, 's'},
        {"end"       , required_argument, 0, 'e'},
        {"plaintext" , required_argument, 0, 'p'},
        {"ciphertext", required_argument, 0, 'c'},
        {0      , 0                , 0,  0 }
    };

    setbuf(stdout, NULL);

    while (1) {
        int c = getopt_long(argc, argv, "s:e:p:c:", longopts, NULL);
        if (c < 0) {
            break;
        }
        switch (c) {
        case 's':
            k2start = atoi(optarg);
            break;
        case 'e':
            k2end = atoi(optarg);
            break;
        case 'p':
            plaintextlen = parse_hex_string(optarg, plaintext);
            break;
        case 'c':
            ciphertextlen = parse_hex_string(optarg, ciphertext);
            break;
        default:
            break;
        }
    }

    if (plaintextlen < 13) {
        fprintf(stderr, "Need at least 13 bytes of plaintext.\n");
        exit(1);
    }

    if (ciphertextlen != plaintextlen) {
        fprintf(stderr, "Need %d bytes of ciphertext.\n", plaintextlen);
        exit(1);
    }

    return crack(plaintext, ciphertext, plaintextlen, k2start, k2end);
}

int parse_hex_string(const char *s, unsigned char *out)
{
    int i;

    for (i = 0; i < MAX_BYTES; i++) {
        char num[3];

        if (s[0] == '\0') {
            break;
        }

        if (s[1] == '\0') {
            fprintf(stderr, "Hex string has an odd number of digits; ignoring last digit.\n");
            break;
        }

        num[0] = s[0];
        num[1] = s[1];
        num[2] = '\0';

        out[i] = (unsigned char)strtoul(num, 0, 16);

        s += 2;
    }

    return i;
}

int crack(unsigned char *plaintext, unsigned char *ciphertext, int len, int k2start, int k2end)
{
    int i;
    unsigned char k3[MAX_BYTES];
    int numk2;
    int k2count;
    unsigned int *k2;
    struct flow_context context;

    assert(len > 12);
    assert(len < MAX_BYTES);

    setup();

    context.p = plaintext;
    context.c = ciphertext;

    for (i = 0; i < len; i++) {
        k3[i] = plaintext[i] ^ ciphertext[i];
    }

    k2 = k2_partial_candidates(k3 + 12, len - 12, &numk2);

    if (k2start < 0) {
        k2start = 0;
    } else if (k2start > numk2) {
        k2start = numk2;
    }

    if (k2end < 0 || k2end > numk2) {
        k2end = numk2;
    }

    k2count = k2end - k2start;

    for (i = k2start; i < k2end; i++) {
        int k2searched = i - k2start;
        if ((k2searched % (k2count / 100)) == 0) {
            printf("[%7d..%7d] %7d/%7d candidates searched (%3d%%)\n",
                   k2start, k2end, k2searched, k2count, (k2searched * 100) / k2count);
        }

        k2_candidate_lists(k2[i], k3, 12, &context, got_k2_candidate_list);
    }

    return 0;
}

void got_k2_candidate_list(void *context, unsigned int *k2, int len)
{
    unsigned char k1msb[MAX_BYTES];
    struct flow_context *c = (struct flow_context *)context;

    c->k2 = k2;
    c->k2len = len;

#ifdef EXAMPLE
    /*
     * Since we provide verbose output, it's a good idea to remove
     * most false K2 candidate lists.
     */
    if ((c->k2[0] >> 16) != 0x685D) {
        return;
    }
    if (c->k2[1] != 0x62B540E8) {
        return;
    }
    if (c->k2[2] != 0xB1695C64) {
        return;
    }
    if (c->k2[3] != 0xA2D6182E) {
        return;
    }
#endif

    k1_msbs(k2 + 1, len - 1, k1msb);
    k1_candidate_lists(k1msb, len - 2, c, got_k1_candidate_list);
}

void got_k1_candidate_list(void *context, unsigned int *k1, int len)
{
    int i;
    unsigned char k0lsb[MAX_BYTES];
    unsigned int k0[5];
    struct flow_context *c = (struct flow_context *)context;
    struct keys s;

#ifdef EXAMPLE
    /*
     * Since we provide verbose output, it's a good idea to remove
     * most false K1 candidate lists.
     */
    if (k1[1] != 0x4743C6EE) {
        return;
    }
#endif

    c->k1 = k1;
    c->k1len = len;

    k0_lsbs(k1 + 1, len - 1, k0lsb);
    k0[0] = k0_recover4(k0lsb, &c->p[4]);

#ifdef EXAMPLE
    list_dump(0, c->k2, c->k2len);

    printf("         ");
    printf("         ");
    list_dump(0, c->k1, c->k1len);

    printf("         ");
    printf("         ");
    printf("         ");
    printf("         ");
    for (i = 0; i < len - 2; i++) {
        printf("      %02X%c", k0lsb[i], i == len - 3 ? '\n' : ' ');
    }

    printf("         ");
    printf("         ");
    printf("         ");
    printf("         ");
    printf("         ");
    printf("         ");
    printf("         ");
    printf("         ");
    printf("%08X\n", k0[0]);
#endif

    for (i = 0; i < 5; i++) {
        if (((unsigned char)k0[i]) != k0lsb[i + 4]) {
            return;
        }
        if (i < 4) {
            k0[i + 1] = crc32(k0[i], c->p[i + 8]);
        }
    }

    /* Looks like we found it! */
    s.k0 = k0[0];
    s.k1 = c->k1[6];
    s.k2 = c->k2[8];
    keys_roll_back(&s, c->c, 8);

    printf("k0=%08X k1=%08X k2=%08X\n", s.k0, s.k1, s.k2);
}
