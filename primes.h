/*
 * $Id: primes.h,v 1.4 2001/11/06 22:25:40 jon Exp $
 *
 * Prime power manipulation for meataxe
 *
 */

#ifndef included__primes
#define included__primes

typedef int (*rep_fn)(unsigned int *);

typedef unsigned int (*unary_fn)(unsigned int);

typedef struct {
  rep_fn prime_rep;
  rep_fn decimal_rep;
  unary_fn negate;
  unary_fn invert;
} prime_ops, *prime_opsp;

extern int primes_init(unsigned int prime, prime_opsp ops);

extern int is_a_prime(unsigned int);

extern int is_a_prime_power(unsigned int);

#endif
