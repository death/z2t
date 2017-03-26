#ifndef Z2T_INC_COMMON_H
#define Z2T_INC_COMMON_H

void setup(void);
void k3_to_temp_list(unsigned char k3, unsigned short *temps);

unsigned int crc32(unsigned int prevcrc, unsigned char c);
unsigned int crc32i(unsigned char nextcrc, unsigned char c);

#endif // Z2T_INC_COMMON_H
