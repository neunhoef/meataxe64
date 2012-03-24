/*
 * $Id: nwrite.c,v 1.1 2012/03/24 13:32:21 jon Exp $
 *
 * Write a meataxe 64 header
 *
 */

#include "nwrite.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "nheader.h"
#include "utils.h"
#include "endian.h"

int write_text_nheader(FILE *fp, const nheader *h, const char *name)
{
  u64 prime, nod, noc, nor;

  assert(NULL != h);
  assert(NULL != fp);
  assert(NULL != name);
  NOT_USED(name);
  prime = nheader_get_prime(h);
  nod = nheader_get_nod(h);
  nor = nheader_get_nor(h);
  noc = nheader_get_noc(h);
  if (1 != prime) {
    fprintf(fp, "%2" U64_F, nod);
  } else {
    fprintf(fp, "12");
  }
  fprintf(fp, " %" U64_F "%" U64_F "%" U64_F "\n", prime, nor, noc);
  return 1;
}

int write_binary_nheader(FILE *fp, const nheader *h, const char *file, const char *name)
{
  u64 prime;
  u64 nor;
  u64 noc;

  assert(NULL != h);
  assert(NULL != fp);
  assert(NULL != name);
  assert(NULL != file);
  prime = nheader_get_raw_prime(h);
  nor = nheader_get_nor(h);
  noc = nheader_get_noc(h);

  errno = 0;
  if (1 != endian_write_u64(prime, fp) ||
      1 != endian_write_u64(nor, fp) ||
      1 != endian_write_u64(noc, fp)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to write nheader to binary output %s\n", name, file);
    return 0;
  }
  return 1;
}

int open_and_write_binary_nheader(FILE **outp, const nheader *h, const char *m, const char *name)
{
  FILE *out;
  int res;
  assert(NULL != outp);
  assert(NULL != h);
  assert(NULL != m);
  errno = 0;
  out = fopen64(m, "w+b");
  if (NULL == out) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, m);
    return 0;
  }
  *outp = out;
  res = write_binary_nheader(out, h, m, name);
  if (0 == res) {
    fclose(out);
    *outp = NULL;
  } else {
    *outp = out;
  }
  return res;
}
