/*
 * $Id: utils.h,v 1.8 2001/10/03 00:01:42 jon Exp $
 *
 * Utilities for meataxe
 *
 */

#ifndef included__utils
#define included__utils

#include <stdlib.h>
#include <stdio.h>

#define NOT_USED(_x) (void)(_x)

extern int is_a_prime_power(unsigned int);

extern int read_decimal(const char *str, unsigned int len, unsigned int *out);

extern unsigned int bits_in_unsigned_int;

extern unsigned int digits_of(unsigned int n);

extern unsigned int bits_of(unsigned int n);

extern int my_isspace(int);

extern void *my_malloc(size_t);

unsigned int getin(FILE *f, unsigned int a);

const char *get_str(FILE *f);

#endif
