/*
 * $Id: utils.h,v 1.2 2001/08/30 18:31:45 jon Exp $
 *
 * Utilities for meataxe
 *
 */

#ifndef included__utils
#define included__utils

#define NOT_USED(_x) (void)(_x)

extern int is_a_prime_power(unsigned int);

extern int alloc_matrix(unsigned int nob, unsigned int noc,
                         unsigned int nor, unsigned int **res);

extern unsigned int bits_in_unsigned_int;

#endif
