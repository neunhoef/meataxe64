/*
 * $Id: read.c,v 1.3 2001/08/30 23:13:38 jon Exp $
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

/* Read an unsigned of length at most len */

static unsigned int read_decimal(const char *str, unsigned int len, const char *name)
{
  unsigned int res = 0;

  assert(0 < len);
  assert(len <= 6);
  while (isspace(*str) && len > 0) {
    len--;
    str++;
  }
  while (len > 0) {
    if (isdigit(*str)) {
      res = res*10 + (*str-'0');
      str++;
      len--;
    } else {
      fprintf(stderr, "Unrecognised digit '%c' in header of %s, terminating\n", *str, name);
      exit(1);
    }
  }
  return res;
}

/* Compute the number of bits needed to represent 0 - n-1 */
static unsigned int bits_of(unsigned int n)
{
  unsigned int i = 0;
  assert(n >= 1);
  n -= 1;
  while(n > 0) {
    n >>= 1;
    i++;
  }
  return i;
}

/* Compute the number of decimal digits needed to represent 0 - n-1 */
static unsigned int digits_of(unsigned int n)
{
  unsigned int i = 0;
  assert(n >= 1);
  n -= 1;
  while(n > 0) {
    n /= 10;
    i++;
  }
  return i;
}

int read_text_header(const FILE *fp, header *hp, const char *name)
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
  j = fread(str, 1, LINE_LENGTH, (FILE *)fp);
  if (LINE_LENGTH != j) {
    fprintf(stderr, "End of file reading '%s', terminating\n", name);
    exit(1);
  }
  i = fgetc((FILE *)fp);
  while (i >= 0 && '\n' != (char)i) {
    if (isspace(i))
      i = fgetc((FILE *)fp);
    else
      break;
  }
  if ('\n' != (char)i) {
    fprintf(stderr, "Newline expected reading '%s', terminating\n", name);
    exit(1);
  }
  nod = read_decimal(str, 2, name);
  prime = read_decimal(str+2, 6, name);
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "Prime power expected, found %d reading %s, terminating\n", prime, name);
    exit(1);
  }
  nor = read_decimal(str+2+6, 6, name);
  noc = read_decimal(str+2+6+6, 6, name);
  nob = bits_of(prime);
  *hp = header_create(prime, nob, nod, noc, nor);
  return 1;
}

int read_binary_header(const FILE *fp, header *hp, const char *name)
{
  header h;
  unsigned int nob;
  unsigned int nod;
  unsigned int prime;
  unsigned int nor;
  unsigned int noc;
  
  assert(NULL != hp);
  assert(NULL != fp);
  assert(NULL != name);
  h = header_alloc();
  if (1 != fread(&nob, sizeof(unsigned int), 1, (FILE *)fp) ||
      1 != fread(&prime, sizeof(unsigned int), 1, (FILE *)fp) ||
      1 != fread(&nor, sizeof(unsigned int), 1, (FILE *)fp) ||
      1 != fread(&noc, sizeof(unsigned int), 1, (FILE *)fp)) {
    fprintf(stderr, "Failed to read header from binary input %s\n", name);
    exit(1);
  }
  if (0 == is_a_prime_power(prime)) {
    fprintf(stderr, "Prime power expected, found %d reading %s, terminating\n", prime, name);
    exit(1);
  }
  nod = digits_of(prime);
  header_set_nob(h, nob);
  header_set_nod(h, nod);
  header_set_prime(h, prime);
  header_set_nor(h, nor);
  header_set_noc(h, noc);
  *hp = h;
  return 1;
}

