/*
 * $Id: primes.h,v 1.2 2001/08/30 18:31:45 jon Exp $
 *
 * Prime power manipulation for meataxe
 *
 */

#ifndef included__primes
#define included__primes

/* Convert an integer to its representation in GF(p) */
extern int prime_rep(unsigned int *, unsigned int);

/* Convert an element of GF(p) to its representation integer */
extern int decimal_rep(unsigned int *, unsigned int);

#endif
