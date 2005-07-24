/*
 * $Id: endian.c,v 1.13 2005/07/24 11:31:35 jon Exp $
 *
 * Endian handling for meataxe
 *
 */

#include "endian.h"
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "utils.h"

u32 endian_invert_u32(u32 a)
{
  u32 b = 0;
  u32 i;
  for (i = 0; i < 4; i++) {
    b = (b << 8) | (a & 0xff);
    a >>= 8;
  }
  return b;
}

static int endian_is_big = 0;

int endian_write_word(word a, FILE *fp)
{
  if (endian_is_big) {
    u32 i;
    for (i = 0; i < sizeof(word); i++) {
      unsigned char c = a & 0xff;
      if (1 != fwrite(&c, sizeof(unsigned char), 1, fp))
        return 0;
      a >>= CHAR_BIT;
    }
  } else {
    return (1 == fwrite(&a, sizeof(word), 1, fp));
  }
  return 1;
}

static word endian_get_word(u32 i, const word *row)
{
  if (endian_is_big) {
    u32 j;
    word res = 0;
    const unsigned char *r = (const unsigned char *)(row + i);
    for (j = 0; j < sizeof(word); j++) {
      res |= r[j] << (j * CHAR_BIT);
    }
    return res;
  } else {
    return row[i];
  }
}

int endian_read_word(word *a, FILE *fp)
{
  assert(NULL != a);
  if (endian_is_big) {
    word i;
    if (1 != fread(&i, sizeof(word), 1, fp))
      return 0;
    *a = endian_get_word(0, &i);
    return 1;
  } else {
    return (1 == fread(a, sizeof(word), 1, fp));
  }
}

int endian_write_u32(u32 a, FILE *fp)
{
  if (endian_is_big) {
    u32 i;
    for (i = 0; i < sizeof(u32); i++) {
      unsigned char c = a & 0xff;
      if (1 != fwrite(&c, sizeof(unsigned char), 1, fp))
        return 0;
      a >>= CHAR_BIT;
    }
  } else {
    return (1 == fwrite(&a, sizeof(u32), 1, fp));
  }
  return 1;
}

int endian_write_u64(u64 a, FILE *fp)
{
  if (endian_is_big) {
    u32 i;
    for (i = 0; i < sizeof(u64); i++) {
      unsigned char c = a & 0xff;
      if (1 != fwrite(&c, sizeof(unsigned char), 1, fp))
        return 0;
      a >>= CHAR_BIT;
    }
  } else {
    return (1 == fwrite(&a, sizeof(u64), 1, fp));
  }
  return 1;
}

static u32 endian_get_u32(u32 i, const u32 *row)
{
  if (endian_is_big) {
    u32 j;
    u32 res = 0;
    const unsigned char *r = (const unsigned char *)(row + i);
    for (j = 0; j < sizeof(u32); j++) {
      res |= r[j] << (j * CHAR_BIT);
    }
    return res;
  } else {
    return row[i];
  }
}

int endian_read_u32(u32 *a, FILE *fp)
{
  assert(NULL != a);
  if (endian_is_big) {
    u32 i;
    if (1 != fread(&i, sizeof(u32), 1, fp))
      return 0;
    *a = endian_get_u32(0, &i);
    return 1;
  } else {
    return (1 == fread(a, sizeof(u32), 1, fp));
  }
}

static u64 endian_get_u64(u32 i, const u64 *row)
{
  if (endian_is_big) {
    u32 j;
    u64 res = 0;
    const unsigned char *r = (const unsigned char *)(row + i);
    for (j = 0; j < sizeof(u64); j++) {
      res |= r[j] << (j * CHAR_BIT);
    }
    return res;
  } else {
    return row[i];
  }
}

int endian_read_u64(u64 *a, FILE *fp)
{
  assert(NULL != a);
  if (endian_is_big) {
    u64 i;
    if (1 != fread(&i, sizeof(u64), 1, fp))
      return 0;
    *a = endian_get_u64(0, &i);
    return 1;
  } else {
    return (1 == fread(a, sizeof(u64), 1, fp));
  }
}

void endian_skip_row(FILE *fp, u32 len)
{
  fseeko64(fp, len * sizeof(word), SEEK_CUR);
}

int endian_read_row(FILE *fp, word *row, u32 len)
{
#ifndef NDEBUG
  unsigned int word_size =
#ifdef EIGHT_BYTE_WORD
    8
#else
    4
#endif
    ;
#endif
  if (endian_is_big) {
    while (len > 0) {
      u32 j;
      unsigned char buf[sizeof(word)];
      word res = 0;
      assert(word_size == sizeof(word));
      if (sizeof(word) != fread(buf, 1, sizeof(word), fp)) {
        return 0;
      }
      for (j = 0; j < sizeof(word); j++) {
        res |= buf[j] << (j * CHAR_BIT);
      }
      *row = res;
      row++;
      len--;
    }
    return 1;
  } else {
    return len == fread(row, sizeof(word), len, fp);
  }
}

int endian_write_row(FILE *fp, const word *row, u32 len)
{
  if (endian_is_big) {
    while (len > 0) {
      if (0 == endian_write_word(*row, fp)) {
        return 0;
      }
      row++;
      len--;
    }
    return 1;
  } else {
    return len == fwrite(row, sizeof(word), len, fp);
  }
}

int endian_read_matrix(FILE *fp, word **row,
                       u32 len, u32 nor)
{
  u32 i;
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(fp, row[i], len)) {
      return 0;
    }
  }
  return 1;
}

int endian_write_matrix(FILE *fp, word **row,
                        u32 len, u32 nor)
{
  u32 i;
  for (i = 0; i < nor; i++) {
    if (0 == endian_write_row(fp, row[i], len)) {
      return 0;
    }
  }
  return 1;
}

int endian_copy_matrix(FILE *inp, FILE *outp, word *row,
                       u32 len, u32 nor)
{
  u32 i;
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(inp, row, len)) {
      return 0;
    }
    if (0 == endian_write_row(outp, row, len)) {
      return 0;
    }
  }
  return 1;
}

void endian_init(void)
{
  u32 a = 0x10000001;
  endian_is_big = (0x10 == *((char *)&a));
}

int endian_read_u32_row(FILE *fp, u32 *row, u32 len)
{
  if (endian_is_big) {
    while (len > 0) {
      u32 j;
      unsigned char buf[sizeof(u32)];
      u32 res = 0;
      if (sizeof(u32) != fread(buf, 1, sizeof(u32), fp)) {
        return 0;
      }
      for (j = 0; j < sizeof(u32); j++) {
        res |= buf[j] << (j * CHAR_BIT);
      }
      *row = res;
      row++;
      len--;
    }
    return 1;
  } else {
    return len == fread(row, sizeof(u32), len, fp);
  }
}

int endian_write_u32_row(FILE *fp, const u32 *row, u32 len)
{
  if (endian_is_big) {
    while (len > 0) {
      if (0 == endian_write_u32(*row, fp)) {
        return 0;
      }
      row++;
      len--;
    }
    return 1;
  } else {
    return len == fwrite(row, sizeof(u32), len, fp);
  }
}

int endian_read_u64_row(FILE *fp, u64 *row, u32 len)
{
  if (endian_is_big) {
    while (len > 0) {
      u32 j;
      unsigned char buf[sizeof(u64)];
      u64 res = 0;
      if (sizeof(u64) != fread(buf, 1, sizeof(u64), fp)) {
        return 0;
      }
      for (j = 0; j < sizeof(u64); j++) {
        res |= buf[j] << (j * CHAR_BIT);
      }
      *row = res;
      row++;
      len--;
    }
    return 1;
  } else {
    return len == fread(row, sizeof(u64), len, fp);
  }
}

int endian_write_u64_row(FILE *fp, const u64 *row, u32 len)
{
  if (endian_is_big) {
    while (len > 0) {
      if (0 == endian_write_u64(*row, fp)) {
        return 0;
      }
      row++;
      len--;
    }
    return 1;
  } else {
    return len == fwrite(row, sizeof(u64), len, fp);
  }
}
