/*
 * $Id: utils.h,v 1.4 2001/09/04 23:00:12 jon Exp $
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

extern int read_decimal(const char *str, unsigned int len, unsigned int *out);

extern unsigned int bits_in_unsigned_int;

extern unsigned int digits_of(unsigned int n);

extern unsigned int bits_of(unsigned int n);

#endif
