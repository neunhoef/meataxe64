/*
 * $Id: utils.h,v 1.7 2001/09/30 21:49:18 jon Exp $
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

extern void nextline(FILE *f);

const char *get_str(FILE *f, char **name, unsigned int depth);

#endif
