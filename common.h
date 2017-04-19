#ifndef Z2T_INC_COMMON_H
#define Z2T_INC_COMMON_H

void setup(void);

enum {
    MAX_BYTES = 256
};

int parse_hex_bytes(char **argv, unsigned char *out);
void reverse_bytes(unsigned char *bytes, int len);

const unsigned short *temp_candidates(unsigned char k3);
int k2p_partial_candidates(unsigned int k2, unsigned char k3p, unsigned int *out);
unsigned int *k2_partial_candidates(unsigned char *k3, int k3len, int *outlen);
void k1_msbs(unsigned int *k2, int k2len, unsigned char *out);
void k0_lsbs(unsigned int *k1, int k1len, unsigned char *out);
unsigned int k0_recover4(unsigned char *k0lsb, unsigned char *plaintext);

typedef void (*list_receiver)(void *context, unsigned int *list, int len);
void list_dump(void *context, unsigned int *list, int len);

void k2_candidate_lists(unsigned int k2partial, unsigned char *k3, int k3len, void *context, list_receiver receiver);
void k1_candidate_lists(unsigned char *k1msbs, int k1len, void *context, list_receiver receiver);

unsigned int lcg(unsigned int k1);
unsigned int lcgi(unsigned int k1);

unsigned int crc32(unsigned int prevcrc, unsigned char c);
unsigned int crc32i(unsigned int nextcrc, unsigned char c);

struct keys
{
    unsigned int k0;
    unsigned int k1;
    unsigned int k2;
};

void keys_init(struct keys *s, const char *pass);
unsigned char keys_k3(struct keys *s);
void keys_update(struct keys *s, unsigned char c);
void keys_crypt(struct keys *s, unsigned char *out, unsigned char *in, int size);

void keys_roll_back(struct keys *s, unsigned char *c, int len);

#endif // Z2T_INC_COMMON_H
