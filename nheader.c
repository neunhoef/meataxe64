/*
 * $Id: nheader.c,v 1.2 2012/03/25 11:53:29 jon Exp $
 *
 * Header manipulation for meataxe 64
 *
 */

#include "nheader.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "primes.h"
#include "utils.h"

struct nheader_struct
{
  u64 prime;	/* The prime(power) in use. 1 for permutations */
  u64 nob;	/* The number of bits per element (binary) */
  u64 nod;	/* The number of digits per element (text) */
  u64 nor;	/* The number of rows */
  u64 noc;	/* The number of columns */
  u64 len;	/* The number of words in a row */
};

u64 nheader_get_prime(const nheader *h)
{
  assert(NULL != h);
  return h->prime & PRIME_MASK;
}

u64 nheader_get_raw_prime(const nheader *h)
{
  assert(NULL != h);
  return h->prime;
}

void nheader_set_prime(nheader *h, u64 p)
{
  assert(NULL != h);
  assert(1 == p || is_a_prime_power(p));
  h->prime = p;
}

void nheader_set_nob(nheader *h, u64 n)
{
  assert(NULL != h);
  assert(n >= 1 || 1 == h->prime);
  h->nob = n;
}

u64 nheader_get_nod(const nheader *h)
{
  assert(NULL != h);
  return h->nod;
}

void nheader_set_nod(nheader *h, u64 n)
{
  assert(NULL != h);
  assert(n >= 1 || 1 == h->prime);
  h->nod = n;
}

u64 nheader_get_nor(const nheader *h)
{
  assert(NULL != h);
  return h->nor;
}

void nheader_set_nor(nheader *h, u64 n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->nor = n;
}

u64 nheader_get_noc(const nheader *h)
{
  assert(NULL != h);
  return h->noc;
}

void nheader_set_noc(nheader *h, u64 n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->noc = n;
}

void nheader_set_len(nheader *h)
{
  assert(NULL != h);
  assert(0 != h->nob || 1 == h->prime);
  h->len = nheader_compute_len(h->nob, h->noc);
}

u64 nheader_compute_len(u64 nob, u64 noc)
{
  if (0 == nob) {
    return 0;
  } else {
    u64 elts_in_word = bits_in_word / nob;
    return (noc + elts_in_word - 1) / elts_in_word;
  }
}

int nheader_alloc(nheader **h)
{
  nheader *h1 = my_malloc(sizeof(*h1));
  assert(NULL != h);
  if (NULL == h1) {
    fprintf(stderr, "Failed to allocate nheader structure\n");
    return 0;
  }
  *h = h1;
  return 1;
}

void nheader_free(const nheader *h)
{
  assert(NULL != h);
  memset((nheader *)h, 0x4c, sizeof(*h));
  free((nheader *)h);
}

nheader *nheader_create(u64 prime, u64 nob,
                      u64 nod, u64 noc,
                      u64 nor)
{
  nheader *h = NULL;
  int i = nheader_alloc(&h);
  assert(NULL != h);
  assert(i);
  NOT_USED(i);
  nheader_set_prime(h, prime);
  nheader_set_nob(h, nob);
  nheader_set_nod(h, nod);
  nheader_set_noc(h, noc);
  nheader_set_nor(h, nor);
  nheader_set_len(h);
  return h;
}
