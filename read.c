/*
 * $Id: read.c,v 1.8 2001/09/16 10:05:44 jon Exp $
 *
 * Read a header
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "header.h"
#include "utils.h"
#include "read.h"

/* Read a header out of a file */
/* Leaves file pointer pointing at next line */
/* Assumes file pointer at start of header line */
/* Assumes the file is being read in text mode */
/* Returns result in h */
/* Zero return is failure, not zero is ok */
/* Format is %2d%6d%6d%6d\n with leading spaces */

#define LINE_LENGTH (2+6+6+6)

int read_text_header(FILE *fp, const header **hp, const char *name)
{
  unsigned int nob;
  unsigned int nod;
  unsigned int prime;
  unsigned int nor;
  unsigned int noc;
  char str[LINE_LENGTH+1];
  unsigned int j;
  int i;
  
  assert(NULL != fp);
  assert(NULL != hp);
  assert(NULL != name);
  j = fread(str, 1, LINE_LENGTH, fp);
  if (LINE_LENGTH != j) {
    fprintf(stderr, "End of file reading '%s'\n", name);
    return 0;
  }
  i = fgetc(fp);
  while (i >= 0 && '\n' != (char)i) {
    if (my_isspace(i))
      i = fgetc(fp);
    else
      break;
  }
  if ('\n' != (char)i) {
    fprintf(stderr, "Newline expected reading '%s'\n", name);
    return 0;
  }
  if (0 == read_decimal(str, 2, &nod)) {
    fprintf(stderr, "Failed to read nod from %s\n", name);
    return 0;
  }
  if (0 == read_decimal(str+2, 6, &prime)) {
    fprintf(stderr, "Failed to read prime from %s\n", name);
    return 0;
  }
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "Prime power expected, found %d reading %s\n", prime, name);
    return 0;
  }
  if (0 == read_decimal(str+2+6, 6, &nor)) {
    fprintf(stderr, "Failed to read nor from %s\n", name);
    return 0;
  }
  if (0 == read_decimal(str+2+6+6, 6, &noc)) {
    fprintf(stderr, "Failed to read noc from %s\n", name);
    return 0;
  }
  nob = bits_of(prime);
  *hp = header_create(prime, nob, nod, noc, nor);
  return 1;
}

int read_binary_header(FILE *fp, const header **hp, const char *name)
{
  header *h;
  unsigned int nob;
  unsigned int nod;
  unsigned int prime;
  unsigned int nor;
  unsigned int noc;
  
  assert(NULL != hp);
  assert(NULL != fp);
  assert(NULL != name);
  if (0 == header_alloc(&h)) {
    fprintf(stderr, "Failed to allocate header for binary input %s\n", name);
    return 0;
  }
  if (1 != endian_read_int(&nob, fp) ||
      1 != endian_read_int(&prime, fp) ||
      1 != endian_read_int(&nor, fp) ||
      1 != endian_read_int(&noc, fp)) {
    fprintf(stderr, "Failed to read header from binary input %s\n", name);
    return 0;
  }
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "Prime power expected, found %d while reading %s\n", prime, name);
    return 0;
  }
  nod = digits_of(prime);
  header_set_nob(h, nob);
  header_set_nod(h, nod);
  header_set_prime(h, prime);
  header_set_nor(h, nor);
  header_set_noc(h, noc);
  header_set_len(h);
  *hp = h;
  return 1;
}

