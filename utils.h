/*
 * $Id: utils.h,v 1.18 2002/09/01 12:33:40 jon Exp $
 *
 * Utilities for meataxe
 *
 */

#ifndef included__utils
#define included__utils

#include <stdlib.h>
#include <stdio.h>

#define NOT_USED(_x) (void)(_x)

#define MAX_LINE 100000

/* Large files */
extern FILE *fopen64(const char *, const char *);

extern int fseeko64(FILE *, long long, int);

extern long long ftello64(FILE *);

extern unsigned int read_decimal(const char *str, unsigned int len);

extern unsigned int bits_in_unsigned_int;

extern unsigned int digits_of(unsigned int n);

extern unsigned int bits_of(unsigned int n);

extern int my_isspace(int);

extern int my_isdigit(int);

extern void *my_malloc(size_t);

extern unsigned int getin(FILE *f, unsigned int a);

extern const char *get_str(FILE *f);

extern void copy_rest(FILE *new, FILE *old);

extern unsigned int skip_whitespace(unsigned int i, const char *chars);

extern unsigned int skip_non_white(unsigned int i, const char *chars);

extern int get_task_line(char *line, FILE *input);

extern int int_pow(unsigned int n, unsigned int index,
                   unsigned int *res);

/* Swap the bit order in a char */
extern unsigned char convert_char(unsigned char in);

/* Read a file of numbers into an array */

extern int read_numbers(FILE *inp, unsigned int s, unsigned int *out);

#endif
