/*
 * extract - Extract values of interest from a ZIP file.
 *
 * For each encrypted file in the ZIP, the values of interest are its
 * name, its CRC, its compressed and original sizes, the 12 octets
 * making up the (encrypted) encryption header, and 32 initial octets
 * of its encrypted and possibly compressed data, which we assume to
 * exist, for good measure.
 *
 * The output is a bunch of S-expressions.
 *
 * We don't perform a lot of error checking or handling, because it's
 * for losers who err, amirite?  Sanity of the input is also assumed.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void lose(const char *tag);
void do_zip(const char *filename);

enum {
      FILE_ENCRYPTED = 0x00000001
};

struct file_entry
{
  char *name;
  unsigned int flags;
  unsigned int crc;
  unsigned int compressed_size;
  unsigned int uncompressed_size;
  unsigned char enc[12];
  unsigned char initial[32];
};

int read_file_entry(FILE *stream, struct file_entry *e);
void free_file_entry(struct file_entry *e);
void print_file_entry(struct file_entry *e);

unsigned int u32(FILE *stream);
unsigned int u16(FILE *stream);
unsigned char *raw(FILE *stream, int size);
void skip(FILE *stream, int n);

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "usage: extract <zipfile>\n");
    return 1;
  }
  do_zip(argv[1]);
  return 0;
}

void lose(const char *tag)
{
  fprintf(stderr, "%s: lose, lose\n", tag);
  exit(2);
}

void do_zip(const char *filename)
{
  FILE *stream;
  struct file_entry e;

  stream = fopen(filename, "rb");
  if (!stream) {
    lose("do_zip");
  }

  while (read_file_entry(stream, &e) != -1) {
    print_file_entry(&e);
    free_file_entry(&e);
  }

  fclose(stream);
}

int read_file_entry(FILE *stream, struct file_entry *e)
{
  int gpflag;
  int fnsize;
  int efsize;

  /* Reset file entry. */
  e->name = 0;
  e->flags = 0;
  e->crc = 0;
  e->compressed_size = 0;
  e->uncompressed_size = 0;

  /* Check signature. */
  if (u32(stream) != 0x04034B50) {
    return -1;
  }

  /* Check version needed to extract. */
  if (u16(stream) != 0x0014) {
    return -1;
  }

  /*
   * Read general purpose bit flag.
   *  bit 0 - file is encrypted
   *  bit 3 - need values from data descriptor
   */
  gpflag = u16(stream);
  if (gpflag & (1 << 0)) {
    e->flags |= FILE_ENCRYPTED;
  }
  if (gpflag & (1 << 3)) {
    /* TODO: use values in data descriptor. */
    return -1;
  }

  /* Read compression method; 0x0008 = deflate. */
  u16(stream);

  /* Read modification time and date. */
  u16(stream);
  u16(stream);

  /* Read CRC. */
  e->crc = u32(stream);

  /* Read compressed size, including encryption header if exists. */
  e->compressed_size = u32(stream);
  if (e->compressed_size == 0xFFFFFFFF) {
    /* TODO: support zip64. */
    return -1;
  }

  /* Read uncompressed size. */
  e->uncompressed_size = u32(stream);

  /* Read filename and extra field. */
  fnsize = u16(stream);
  efsize = u16(stream);
  e->name = strdup((const char *)raw(stream, fnsize));
  if (efsize > 0) {
    raw(stream, efsize);
  }

  /* Read (encrypted) encryption header if exists. */
  if (gpflag & (1 << 0)) {
    if (e->compressed_size < 12) {
      free_file_entry(e);
      return -1;
    }
    memmove(e->enc, raw(stream, 12), 12);
    e->compressed_size -= 12;
  }

  /*
   * Read 32 octets of the encrypted data, which we assume to
   * exist.
   */
  if (e->compressed_size < 32) {
    free_file_entry(e);
    return -1;
  }
  memmove(e->initial, raw(stream, 32), 32);

  /* Skip the rest of the data. */
  skip(stream, e->compressed_size - 32);

  /*
   * Currently we assume that there's a data directory if and only if
   * bit 3 of gpflag is set.
   */
  if (gpflag & (1 << 3)) {
    u32(stream);
    u32(stream);                  /* compressed size */
    u32(stream);                  /* uncompressed size */
  }

  return 0;
}

void free_file_entry(struct file_entry *e)
{
  free(e->name);
  e->name = 0;
}

void print_file_entry(struct file_entry *e)
{
  int i;

  printf("(");

  printf("name \"%s\" crc #x%08X csize %d usize %d ",
         e->name ? e->name : "",
         e->crc,
         e->compressed_size,
         e->uncompressed_size);

  if (e->flags & FILE_ENCRYPTED) {
    printf("enc #(");
    for (i = 0; i < 12; i++) {
      if (i != 0) {
        printf(" ");
      }
      printf("#x%02X", e->enc[i]);
    }
    printf(") ");
  }

  printf("initial #(");
  for (i = 0; i < 32; i++) {
    if (i != 0) {
      printf(" ");
    }
    printf("#x%02X", e->initial[i]);
  }
  printf(")");

  printf(")\n");
}

unsigned int u32(FILE *stream)
{
  unsigned char *buf = raw(stream, 4);
  return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

unsigned int u16(FILE *stream)
{
  unsigned char *buf = raw(stream, 2);
  return buf[0] | (buf[1] << 8);
}

unsigned char *raw(FILE *stream, int size)
{
  static unsigned char buf[0x10000];
  assert(size >= 0 && size <= 0xFFFF);
  memset(buf, 0, sizeof(buf));
  if (fread(buf, 1, size, stream) != size) {
    lose("raw");
  }
  return buf;
}

void skip(FILE *stream, int n)
{
  if (fseek(stream, n, SEEK_CUR) != 0) {
    lose("skip");
  }
}
