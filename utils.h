/*
 * $Id: utils.h,v 1.6 2001/09/25 22:31:58 jon Exp $
 *
 * Utilities for meataxe
 *
 */

#ifndef included__utils
#define included__utils

#include <stdlib.h>

#define NOT_USED(_x) (void)(_x)

extern int is_a_prime_power(unsigned int);

extern int read_decimal(const char *str, unsigned int len, unsigned int *out);

extern unsigned int bits_in_unsigned_int;

extern unsigned int digits_of(unsigned int n);

extern unsigned int bits_of(unsigned int n);

extern int my_isspace(int);

extern void *my_malloc(size_t);

#endif
