/*
 * $Id: utils.h,v 1.11 2001/11/06 22:25:40 jon Exp $
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

extern unsigned int read_decimal(const char *str, unsigned int len);

extern unsigned int bits_in_unsigned_int;

extern unsigned int digits_of(unsigned int n);

extern unsigned int bits_of(unsigned int n);

extern int my_isspace(int);

extern void *my_malloc(size_t);

extern unsigned int getin(FILE *f, unsigned int a);

extern const char *get_str(FILE *f);

extern void copy_rest(FILE *new, FILE *old);

extern unsigned int skip_whitespace(unsigned int i, const char *chars);

extern unsigned int skip_non_white(unsigned int i, const char *chars);

extern int get_task_line(char *line, FILE *input);

#endif
