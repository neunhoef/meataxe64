/*
 * $Id: primes.h,v 1.3 2001/09/05 22:47:25 jon Exp $
 *
 * Prime power manipulation for meataxe
 *
 */

#ifndef included__primes
#define included__primes

typedef int (*rep_fn)(unsigned int *);

typedef struct {
  rep_fn prime_rep;
  rep_fn decimal_rep;
} prime_ops, *prime_opsp;

extern int primes_init(unsigned int prime, prime_opsp ops);

#endif
