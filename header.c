/*
 * $Id: header.c,v 1.1 2001/08/28 21:39:44 jon Exp $
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
};

unsigned int header_get_prime(header h)
{
  assert(NULL != h);
  return h->prime;
}

void header_set_prime(header h, unsigned int p)
{
  assert(NULL != h);
  assert(is_a_prime_power(p));
  h->prime = p;
}

unsigned int header_get_nob(header h)
{
  assert(NULL != h);
  return h->nob;
}

void header_set_nob(header h, unsigned int n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->nob = n;
}

unsigned int header_get_nod(header h)
{
  assert(NULL != h);
  return h->nod;
}

void header_set_nod(header h, unsigned int n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->nod = n;
}

unsigned int header_get_nor(header h)
{
  assert(NULL != h);
  return h->nor;
}

void header_set_nor(header h, unsigned int n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->nor = n;
}

unsigned int header_get_noc(header h)
{
  assert(NULL != h);
  return h->noc;
}

void header_set_noc(header h, unsigned int n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->noc = n;
}

header header_alloc(void)
{
  header h = malloc(sizeof(*h));
  if (NULL == h) {
    fprintf(stderr, "Failed to allocate header structure, terminating\n");
    exit(1);
  }
  return h;
}

void header_free(header h)
{
  assert(NULL != h);
  memset(h, 0x4c, sizeof(*h));
  free(h);
}

header header_create(unsigned int prime, unsigned int nob,
                     unsigned int nod, unsigned int noc,
                     unsigned int nor)
{
  header h = header_alloc();
  assert(NULL != h);
  header_set_prime(h, prime);
  header_set_nob(h, nob);
  header_set_nod(h, nod);
  header_set_noc(h, noc);
  header_set_nor(h, nor);
  return h;
}
