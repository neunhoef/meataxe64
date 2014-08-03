/*
 * $Id: header.c,v 1.19 2014/08/03 11:56:42 jon Exp $
 *
 * Header manipulation
 *
 */

#include "header.h"
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "primes.h"
#include "utils.h"

struct header_struct
{
  u32 prime;	/* The prime(power) in use. 1 for permutations */
  u32 nob;	/* The number of bits per element (binary) */
  u32 nod;	/* The number of digits per element (text) */
  u32 nor;	/* The number of rows */
  u32 noc;	/* The number of columns */
  u32 len;	/* The number of words in a row */
  u32 blen;	/* The number of bytes required for a row for old meataxe */
  u32 eperb;	/* The number of elements per byte for old meataxe */
};

u32 header_get_prime(const header *h)
{
  assert(NULL != h);
  return h->prime & PRIME_MASK;
}

u32 header_get_raw_prime(const header *h)
{
  assert(NULL != h);
  return h->prime;
}

void header_set_prime(header *h, u32 p)
{
  assert(NULL != h);
  assert(1 == p || is_a_prime_power(p));
  h->prime = p | PRIME_BIT;
}

void header_set_raw_prime(header *h, u32 p)
{
  assert(NULL != h);
  h->prime = p;
}

u32 header_get_nob(const header *h)
{
  assert(NULL != h);
  return h->nob;
}

void header_set_nob(header *h, u32 n)
{
  assert(NULL != h);
  assert(n >= 1 || 1 == h->prime);
  h->nob = n;
}

u32 header_get_nod(const header *h)
{
  assert(NULL != h);
  return h->nod;
}

void header_set_nod(header *h, u32 n)
{
  assert(NULL != h);
  assert(n >= 1 || 1 == h->prime);
  h->nod = n;
}

u32 header_get_nor(const header *h)
{
  assert(NULL != h);
  return h->nor;
}

void header_set_nor(header *h, u32 n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->nor = n;
}

u32 header_get_noc(const header *h)
{
  assert(NULL != h);
  return h->noc;
}

void header_set_noc(header *h, u32 n)
{
  assert(NULL != h);
  assert(n >= 1);
  h->noc = n;
}

u32 compute_len(u32 nob, u32 noc)
{
  if (0 == nob) {
    return 0;
  } else {
    u32 elts_in_word = bits_in_word / nob;
    return (noc + elts_in_word - 1) / elts_in_word;
  }
}

static u32 compute_u32_len(u32 nob, u32 noc)
{
  if (0 == nob) {
    return 0;
  } else {
    u32 elts_in_u32 = bits_in_u32 / nob;
    return (noc + elts_in_u32 - 1) / elts_in_u32;
  }
}

static u32 compute_u64_len(u32 nob, u32 noc)
{
  if (0 == nob) {
    return 0;
  } else {
    u32 elts_in_u64 = bits_in_u64 / nob;
    return (noc + elts_in_u64 - 1) / elts_in_u64;
  }
}

u32 header_get_len(const header *h)
{
  assert(NULL != h);
  assert(compute_len(h->nob, h->noc) == h->len);
  return h->len;
}

u32 header_get_u32_len(const header *h)
{
  assert(NULL != h);
  return compute_u32_len(h->nob, h->noc);
}

u32 header_get_u64_len(const header *h)
{
  assert(NULL != h);
  return compute_u64_len(h->nob, h->noc);
}

void header_set_len(header *h)
{
  assert(NULL != h);
  assert(0 != h->nob || 1 == h->prime);
  h->len = compute_len(h->nob, h->noc);
}

static u32 get_eperb(u32 prime, u32 nob)
{
  prime &= PRIME_MASK;
  if (1 == prime) {
    return 1;
  }
  if (0 == prime % 2) {
    return (CHAR_BIT) / nob;
  } else {
    double log2 = log(2);
    double logq = log(prime);
    return (u32)((CHAR_BIT) * log2 / logq);
  }
}

u32 header_get_eperb(const header *h)
{
  assert(NULL != h);
  assert(get_eperb(h->prime, h->nob) == h->eperb);
  return h->eperb;
}

void header_set_eperb(header *h)
{
  assert(NULL != h);
  assert(0 != h->nob || 1 == h->prime);
  h->eperb = get_eperb(h->prime, h->nob);
}

static u32 get_blen(const header *h)
{
  assert(NULL != h);
  if (1 == h->prime) {
    return 0;
  } else {
    assert(get_eperb(h->prime, h->nob) == h->eperb);
    return (h->noc + h->eperb - 1) / h->eperb;
  }
}

u32 header_get_blen(const header *h)
{
  assert(NULL != h);
  assert(get_blen(h) == h->blen);
  return h->blen;
}

void header_set_blen(header *h)
{
  assert(NULL != h);
  assert(0 != h->nob || 1 == h->prime);
  h->blen = get_blen(h);
}

int header_alloc(header **h)
{
  header *h1 = my_malloc(sizeof(*h1));
  assert(NULL != h);
  if (NULL == h1) {
    fprintf(stderr, "Failed to allocate header structure\n");
    return 0;
  }
  *h = h1;
  return 1;
}

void header_free(const header *h)
{
  assert(NULL != h);
  memset((header *)h, 0x4c, sizeof(*h));
  free((header *)h);
}

header *header_create(u32 prime, u32 nob,
                      u32 nod, u32 noc,
                      u32 nor)
{
  header *h = NULL;
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
  header_set_eperb(h);
  header_set_blen(h);
  return h;
}

int header_check(const header *h)
{
  u32 prime_bits;
  assert(NULL != h);
  prime_bits = h->prime & PRIME_BITS;
  if (prime_bits == PRIME_BIT) {
    return 1;
  } else {
    return 0;
  }
}
