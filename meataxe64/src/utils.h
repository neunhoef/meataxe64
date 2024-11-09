/*
 * $Id: utils.h,v 1.1 2017/10/02 20:01:00 jon Exp $
 *
 * Utilities for meataxe
 *
 */

#ifndef included__utils
#define included__utils

#include <stdlib.h>
#include <stdio.h>
#include "mt_types.h"

#define NOT_USED(_x) (void)(_x)

#define MAX_LINE 100000

/* Large files */
extern u32 read_decimal(const char *str, u32 len);

extern u32 bits_in_word;

extern u32 bits_in_u32;

extern u32 bits_in_u64;

extern u32 digits_of(u32 n);

extern u32 bits_of(u32 n);

extern int my_isspace(int);

extern int my_isdigit(int);

extern void *my_malloc(size_t);

extern u32 getin(FILE *f, u32 a);

extern const char *get_str(FILE *f);

extern void copy_rest(FILE *new, FILE *old);

extern void copy_file(const char *new, const char *old);

extern u32 skip_whitespace(u32 i, const char *chars);

extern u32 skip_non_white(u32 i, const char *chars);

extern int get_task_line(char *line, FILE *input);

extern int int_pow(u32 n, u32 index,
                   u32 *res);

/* Swap the bit order in a char */
extern unsigned char convert_char(unsigned char in);

/* Read a file of numbers into an array */

extern int read_numbers(FILE *inp, u32 s, u32 *out);

/* A value to avoid assertions on constant expressions */

extern int assert_var_zero;

#endif
