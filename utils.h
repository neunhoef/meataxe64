/*
 * $Id: utils.h,v 1.3 2001/09/02 22:16:41 jon Exp $
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

extern unsigned int read_decimal(const char *str, unsigned int len);

extern unsigned int bits_in_unsigned_int;

extern unsigned int digits_of(unsigned int n);

extern unsigned int bits_of(unsigned int n);

#endif
