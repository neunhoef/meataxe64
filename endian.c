/*
 * $Id: endian.c,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Endian handling for meataxe
 *
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#if 0
#include <stdlib.h>
#endif
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

void endian_init(void)
{
  unsigned int a = 0x10000001;
  endian_is_big = (0x10 == *((char *)&a));
}

