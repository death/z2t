#ifndef Z2T_INC_COMMON_H
#define Z2T_INC_COMMON_H

void setup(void);

const unsigned short *temp_candidates(unsigned char k3);
int k2p_candidates(unsigned int k2, unsigned char k3p, unsigned int *out);

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

#endif // Z2T_INC_COMMON_H
