/*
 * $Id: endian.c,v 1.5 2001/09/16 10:05:44 jon Exp $
 *
 * Endian handling for meataxe
 *
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "utils.h"
#include "endian.h"

static int endian_is_big = 0;

int endian_write_int(unsigned int a, FILE *fp)
{
  if (endian_is_big) {
    int i;
    for (i = 0; i < 4; i++) {
      unsigned char c = a & 0xff;
      if (1 != fwrite(&c, sizeof(unsigned char), 1, fp))
        return 0;
      a >>= CHAR_BIT;
    }
  } else {
    return (1 == fwrite(&a, sizeof(unsigned int), 1, fp));
  }
  return 1;
}

int endian_read_int(unsigned int *a, FILE *fp)
{
  assert(NULL != a);
  if (endian_is_big) {
    unsigned int i;
    if (1 != fread(&i, sizeof(unsigned int), 1, fp))
      return 0;
    *a = endian_get_int(0, &i);
    return 1;
  } else {
    return (1 == fread(a, sizeof(unsigned int), 1, fp));
  }
  return 1;
}

unsigned int endian_get_int(unsigned int i, const unsigned int *row)
{
  if (endian_is_big) {
    int j;
    unsigned int res = 0;
    const unsigned char *r = (const unsigned char *)(row + i);
    for (j = 0; j < 4; j++) {
      res |= r[j] << (j * CHAR_BIT);
    }
    return res;
  } else {
    return row[i];
  }
}

int endian_read_row(FILE *fp, unsigned int *row, unsigned int len)
{
  unsigned int row_words = len / sizeof(unsigned int);
  if (endian_is_big) {
    while (row_words > 0) {
      int j;
      unsigned char buf[4];
      unsigned int res = 0;
      assert(4 == sizeof(unsigned int));
      if (4 != fread(buf, 1, 4, fp)) {
        return 0;
      }
      for (j = 0; j < 4; j++) {
        res |= buf[j] << (j * CHAR_BIT);
      }
      *row = res;
      row++;
      row_words--;
    }
    return 1;
  } else {
    return row_words == fread(row, sizeof(unsigned int), row_words, fp);
  }
}

int endian_write_row(FILE *fp, const unsigned int *row, unsigned int len)
{
  unsigned int row_words = len / sizeof(unsigned int);
  if (endian_is_big) {
    while (row_words > 0) {
      endian_write_int(*row, fp);
      row++;
      row_words--;
    }
    return 1;
  } else {
    return row_words == fwrite(row, sizeof(unsigned int), row_words, fp);
  }
}

int endian_read_matrix(FILE *fp, unsigned int **row,
                       unsigned int len, unsigned int nor)
{
  unsigned int i;
  for (i = 0; i < nor; i++) {
    if (0 == endian_read_row(fp, row[i], len)) {
      return 0;
    }
  }
  return 1;
}

int endian_write_matrix(FILE *fp, unsigned int **row,
                        unsigned int len, unsigned int nor)
{
  unsigned int i;
  for (i = 0; i < nor; i++) {
    if (0 == endian_write_row(fp, row[i], len)) {
      return 0;
    }
  }
  return 1;
}

void endian_init(void)
{
  unsigned int a = 0x10000001;
  endian_is_big = (0x10 == *((char *)&a));
}
