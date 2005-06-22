/*
 * $Id: write.c,v 1.14 2005/06/22 21:52:54 jon Exp $
 *
 * Write a header
 *
 */

#include "write.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "header.h"
#include "utils.h"
#include "endian.h"

int write_text_header(FILE *fp, const header *h, const char *name)
{
  u32 prime, nod, noc, nor;

  assert(NULL != h);
  assert(NULL != fp);
  assert(NULL != name);
  NOT_USED(name);
  prime = header_get_prime(h);
  nod = header_get_nod(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  if (1 != prime) {
    fprintf(fp, "%2d", nod);
  } else {
    fprintf(fp, "12");
  }
  fprintf(fp, " %d %d %d\n", prime, nor, noc);
  return 1;
}

int write_binary_header(FILE *fp, const header *h, const char *file, const char *name)
{
  u32 prime;
  u32 nor;
  u32 noc;

  assert(NULL != h);
  assert(NULL != fp);
  assert(NULL != name);
  assert(NULL != file);
  prime = header_get_raw_prime(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);

  errno = 0;
  if (1 != endian_write_u32(prime, fp) ||
      1 != endian_write_u32(nor, fp) ||
      1 != endian_write_u32(noc, fp)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to write header to binary output %s\n", name, file);
    return 0;
  }
  return 1;
}

int open_and_write_binary_header(FILE **outp, const header *h, const char *m, const char *name)
{
  FILE *out;
  int res;
  assert(NULL != outp);
  assert(NULL != h);
  assert(NULL != m);
  errno = 0;
  out = fopen64(m, "wb");
  if (NULL == out) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, m);
    return 0;
  }
  *outp = out;
  res = write_binary_header(out, h, m, name);
  if (0 == res) {
    fclose(out);
    *outp = NULL;
  } else {
    *outp = out;
  }
  return res;
}
