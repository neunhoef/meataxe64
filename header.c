/*
 * $Id: header.c,v 1.5 2001/10/03 23:57:32 jon Exp $
 *
 * Header manipulation
 *
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "header.h"

struct header_struct
{
  unsigned int prime;	/* The prime(power) in use. 1 for permutations */
  unsigned int nob;	/* The number of bits per element (binary) */
  unsigned int nod;	/* The number of digits per element (text) */
  unsigned int nor;	/* The number of rows */
  unsigned int noc;	/* The number of columns */
  unsigned int len;	/* The number of words in a row */
};

unsigned int header_get_prime(const header *h)
{
  assert(NULL != h);
  return h->prime;
}

void header_set_prime(header *h, unsigned int p)
{
  assert(NULL != h);
  assert(is_a_prime_power(p));
  h->prime = p;
}

unsigned int header_get_nob(const header *h)
{
  assert(NULL != h);
  return h->nob;
}

void header_set_nob(header *h, unsigned int n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->nob = n;
}

unsigned int header_get_nod(const header *h)
{
  assert(NULL != h);
  return h->nod;
}

void header_set_nod(header *h, unsigned int n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->nod = n;
}

unsigned int header_get_nor(const header *h)
{
  assert(NULL != h);
  return h->nor;
}

void header_set_nor(header *h, unsigned int n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->nor = n;
}

unsigned int header_get_noc(const header *h)
{
  assert(NULL != h);
  return h->noc;
}

void header_set_noc(header *h, unsigned int n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->noc = n;
}

static unsigned int get_len(unsigned int nob, unsigned int noc)
{
  unsigned int elts_in_word = bits_in_unsigned_int / nob;
  return (noc + elts_in_word - 1) / elts_in_word;
}

unsigned int header_get_len(const header *h)
{
  assert(NULL != h);
  assert(get_len(h->nob, h->noc) == h->len);
  return h->len;
}

void header_set_len(header *h)
{
  assert(NULL != h);
  assert(0 != h->nob);
  h->len = get_len(h->nob, h->noc);
}

int header_alloc(header **h)
{
  header *h1 = malloc(sizeof(*h1));
  assert(NULL != h);
  if (NULL == h1) {
    fprintf(stderr, "Failed to allocate header structure\n");
    return 0;
  }
  *h = h1;
  return 1;
}

void header_free(header *h)
{
  assert(NULL != h);
  memset(h, 0x4c, sizeof(*h));
  free(h);
}

const header *header_create(unsigned int prime, unsigned int nob,
                     unsigned int nod, unsigned int noc,
                     unsigned int nor)
{
  header *h;
  int i = header_alloc(&h);
  assert(NULL != h);
  assert(i);
  NOT_USED(i);
  header_set_prime(h, prime);
  header_set_nob(h, nob);
  header_set_nod(h, nod);
  header_set_noc(h, noc);
  header_set_nor(h, nor);
  header_set_len(h);
  return h;
}
