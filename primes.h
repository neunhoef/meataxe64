/*
 * $Id: primes.h,v 1.8 2005/06/22 21:52:53 jon Exp $
 *
 * Prime power manipulation for meataxe
 *
 */

#ifndef included__primes
#define included__primes

typedef int (*rep_fn)(word *);

typedef word (*unary_fn)(word);

typedef word (*binary_fn)(word, word);

typedef struct {
  u32 prime;
  rep_fn prime_rep;
  rep_fn decimal_rep;
  unary_fn negate;
  unary_fn invert;
  binary_fn power;
  binary_fn add;
  binary_fn mul;
} prime_ops, *prime_opsp;

extern int primes_init(u32 prime, prime_opsp ops);

extern int is_a_prime(word);

extern int is_a_prime_power(word);

extern word prime_divisor(word q);

extern u32 prime_index(word q, u32 prime);

#endif
