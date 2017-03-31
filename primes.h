/*
 * $Id: primes.h,v 1.10 2017/03/31 19:52:22 jon Exp $
 *
 * Prime power manipulation for meataxe
 *
 */

#ifndef included__primes
#define included__primes

typedef int (*rep_fn)(const word *);

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
  int (*is_invertible)(word);
} prime_ops, *prime_opsp;

extern int primes_init(u32 prime, prime_opsp ops);

extern int is_a_prime(word);

extern int is_a_prime_power(word);

extern word prime_divisor(word q);

extern u32 prime_index(word q, u32 prime);

#endif
