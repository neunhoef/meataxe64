/*
 * $Id: maps.h,v 1.1 2002/04/10 23:33:27 jon Exp $
 *
 * Maps from {0 .. nor-1} -> {0 .. noc-1}
 *
 */

#ifndef included__maps
#define included__maps

#include "header.h"
#include <stdio.h>

extern unsigned int *malloc_map(unsigned int);

extern void map_free(unsigned int *);

extern int read_map(FILE *, unsigned int nor, unsigned int *, const char *name, const char *in);

extern int write_map(FILE *, unsigned int nor, unsigned int *, const char *name, const char *out);

extern int mul_map(unsigned int *in1, unsigned int *in2, unsigned int *out,
                   const header *h1, const header *h2, const char *);

extern int map_rank(FILE *inp, const header *h, const char *m, unsigned int *r, const char *name);

extern int map_iv(FILE *inp, const header *h, const char *m1, const char *m2, const char *name);

extern int read_map_element_as_row(FILE *inp, unsigned int *row, unsigned int nob,
                                   unsigned int noc, unsigned int len, const char *in, const char *name);

#endif
