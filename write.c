/*
 * $Id: write.c,v 1.5 2001/09/16 10:05:44 jon Exp $
 *
 * Write a header
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "header.h"
#include "utils.h"
#include "endian.h"
#include "write.h"

int write_text_header(FILE *fp, const header *h)
{
  unsigned int prime, nod, noc, nor;
  char buf1[12];
  char buf2[12];
  char buf3[12];
  char buf4[12];

  assert(NULL != h);
  assert(NULL != fp);
  prime = header_get_prime(h);
  nod = header_get_nod(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  (void)sprintf(buf1, "%2d", nod);
  (void)sprintf(buf2, "%6d", prime);
  (void)sprintf(buf3, "%6d", nor);
  (void)sprintf(buf4, "%6d", noc);
  if (2 != fwrite(buf1, 1, 2, fp) ||
      6 != fwrite(buf2, 1, 6, fp) ||
      6 != fwrite(buf3, 1, 6, fp) ||
      6 != fwrite(buf4, 1, 6, fp) ||
      fputc('\n', fp) < 0) {
    fprintf(stderr, "Failed to write header to text output\n");
    return 0;
  }
  return 1;
}

int write_binary_header(FILE *fp, const header *h, const char *name)
{
  unsigned int nob;
  unsigned int prime;
  unsigned int nor;
  unsigned int noc;

  assert(NULL != h);
  assert(NULL != fp);
  assert(NULL != name);
  nob = header_get_nob(h);
  prime = header_get_prime(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);

  if (1 != endian_write_int(nob, fp) ||
      1 != endian_write_int(prime, fp) ||
      1 != endian_write_int(nor, fp) ||
      1 != endian_write_int(noc, fp)) {
    fprintf(stderr, "Failed to write header to binary output %s\n", name);
    return 0;
  }
  return 1;
}
