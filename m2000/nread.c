/*
 * $Id: nread.c,v 1.2 2019/01/21 08:32:34 jon Exp $
 *
 * Read a meataxe 64 header
 *
 */

#include "nread.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "nheader.h"
#include "utils.h"
#include "endian.h"

int nread_binary_header(FILE *fp, const nheader **hp, const char *file, const char *name)
{
  nheader *h;
  u64 nob;
  u64 nod;
  u64 prime;
  u64 nor;
  u64 noc;

  assert(NULL != hp);
  assert(NULL != fp);
  assert(NULL != name);
  assert(NULL != file);
  if (0 == nheader_alloc(&h)) {
    fprintf(stderr, "%s: failed to allocate header for binary input %s\n", name, file);
    return 0;
  }
  errno = 0;
  if (1 != endian_read_u64(&prime, fp) ||
      1 != endian_read_u64(&nor, fp) ||
      1 != endian_read_u64(&noc, fp)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read header from binary input %s\n", name, file);
    return 0;
  }
  nod = digits_of((u32)prime);
  nob = bits_of((u32)prime);
  nheader_set_prime(h, prime);
  nheader_set_nob(h, nob);
  nheader_set_nod(h, nod);
  nheader_set_nor(h, nor);
  nheader_set_noc(h, noc);
  nheader_set_len(h);
  *hp = h;
  return 1;
}

int open_and_read_binary_nheader(FILE **inp, const nheader **h, const char *m, const char *name)
{
  FILE *in;
  int res;
  assert(NULL != inp);
  assert(NULL != h);
  assert(NULL != m);
  assert(NULL != name);
  errno = 0;
  in = fopen(m, "rb");
  if (NULL == in) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, m);
    *h = NULL;
    return 0;
  }
  res = nread_binary_header(in, h, m, name);
  if (0 == res) {
    fclose(in);
    *h = NULL;
    *inp = NULL;
  } else {
    *inp = in;
  }
  return res;
}
