/*
 * $Id: maps.h,v 1.2 2005/06/22 21:52:53 jon Exp $
 *
 * Maps from {0 .. nor-1} -> {0 .. noc-1}
 *
 */

#ifndef included__maps
#define included__maps

#include "header.h"
#include <stdio.h>

extern word *malloc_map(u32);

extern void map_free(word *);

extern int read_map(FILE *, u32 nor, word *, const char *name, const char *in);

extern int write_map(FILE *, u32 nor, word *, const char *name, const char *out);

extern int mul_map(word *in1, word *in2, word *out,
                   const header *h1, const header *h2, const char *);

extern int map_rank(FILE *inp, const header *h, const char *m, u32 *r, const char *name);

extern int map_iv(FILE *inp, const header *h, const char *m1, const char *m2, const char *name);

extern int read_map_element_as_row(FILE *inp, word *row, u32 nob,
                                   u32 noc, u32 len, const char *in, const char *name);

#endif
