/*
 * $Id: primes.h,v 1.6 2002/02/05 19:50:56 jon Exp $
 *
 * Prime power manipulation for meataxe
 *
 */

#ifndef included__primes
#define included__primes

typedef int (*rep_fn)(unsigned int *);

typedef unsigned int (*unary_fn)(unsigned int);

typedef unsigned int (*binary_fn)(unsigned int, unsigned int);

typedef struct {
  rep_fn prime_rep;
  rep_fn decimal_rep;
  unary_fn negate;
  unary_fn invert;
  binary_fn power;
  binary_fn add;
  binary_fn mul;
} prime_ops, *prime_opsp;

extern int primes_init(unsigned int prime, prime_opsp ops);

extern int is_a_prime(unsigned int);

extern int is_a_prime_power(unsigned int);

extern unsigned int prime_divisor(unsigned int q);

extern unsigned int prime_index(unsigned int q, unsigned int prime);

#endif
