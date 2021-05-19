/*
 * $Id: read.c,v 1.1 2017/10/02 20:00:59 jon Exp $
 *
 * Read a header
 *
 */

#include "read.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include "endian.h"
#include "header.h"
#include "primes.h"
#include "utils.h"

/* Read a header out of a file */
/* Leaves file pointer pointing at next line */
/* Assumes file pointer at start of header line */
/* Assumes the file is being read in text mode */
/* Returns result in h */
/* Zero return is failure, not zero is ok */
/* Format is %2d%6d%6d%6d\n with leading spaces */

#define LINE_LENGTH (2+6+6+6)

int read_text_header_items(FILE *fp, u32 *nod, u32 *prime,
                           u32 *nor, u32 *noc, const char *file, const char *name)
{
  int i;

  assert(NULL != fp);
  assert(NULL != file);
  assert(NULL != name);
  assert(NULL != nod);
  assert(NULL != prime);
  assert(NULL != nor);
  assert(NULL != noc);
  i = fscanf(fp, "%u%u%u%u", nod, prime, nor, noc);
  NOT_USED(i);
  i = fgetc(fp);
  while (i >= 0 && '\n' != (char)i) {
    if (my_isspace(i)) {
      i = fgetc(fp);
    } else {
      break;
    }
  }
  if ('\n' != (char)i) {
    fprintf(stderr, "%s: newline expected reading '%s'\n", name, file);
    return 0;
  }
  return 1;
}

int read_text_header(FILE *fp, const header **hp, const char *file, const char *name)
{
  u32 nob;
  u32 nod;
  u32 prime;
  u32 nor;
  u32 noc;

  assert(NULL != fp);
  assert(NULL != hp);
  assert(NULL != file);
  assert(NULL != name);
  if (read_text_header_items(fp, &nod, &prime, &nor, &noc, file, name) && (1 == prime || is_a_prime_power(prime))) {
    nob = bits_of(prime);
    *hp = header_create(prime, nob, nod, noc, nor);
    return 1;
  } else {
    fprintf(stderr, "%s: value %u is neither 1 nor a prime\n", name, prime);
    return 0;
  }
}

int read_binary_header(FILE *fp, const header **hp, const char *file, const char *name)
{
  header *h;
  u32 nob;
  u32 nod;
  u32 prime;
  u32 nor;
  u32 noc;
  u32 masked_prime;

  assert(NULL != hp);
  assert(NULL != fp);
  assert(NULL != file);
  assert(NULL != name);
  if (0 == header_alloc(&h)) {
    fprintf(stderr, "%s: failed to allocate header for binary input %s\n", name, file);
    return 0;
  }
  errno = 0;
  if (1 != endian_read_u32(&prime, fp) ||
      1 != endian_read_u32(&nor, fp) ||
      1 != endian_read_u32(&noc, fp)) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: failed to read header from binary input %s\n", name, file);
    return 0;
  }
  masked_prime = prime & PRIME_MASK;
  if (1 != masked_prime && 0 == is_a_prime_power(masked_prime)) {
    fprintf(stderr, "%s: prime power or 1 expected, found %u while reading %s\n", name, masked_prime, file);
    return 0;
  }
  nod = digits_of(masked_prime);
  nob = bits_of(masked_prime);
  header_set_raw_prime(h, prime);
  header_set_nob(h, nob);
  header_set_nod(h, nod);
  header_set_nor(h, nor);
  header_set_noc(h, noc);
  header_set_len(h);
  header_set_eperb(h);
  header_set_blen(h);
  *hp = h;
  return 1;
}

int open_and_read_binary_header(FILE **inp, const header **h, const char *m, const char *name)
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
  res = read_binary_header(in, h, m, name);
  if (0 != res && 0 == header_check(*h)) {
    fprintf(stderr, "%s: expected and found data size mismatch, terminating\n", name);
    res = 0;
  }
  if (0 == res) {
    fclose(in);
    *h = NULL;
    *inp = NULL;
  } else {
    *inp = in;
  }
  return res;
}
