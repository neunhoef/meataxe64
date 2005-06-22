/*
 * $Id: map_or_row.h,v 1.2 2005/06/22 21:52:53 jon Exp $
 *
 * Handle reading from a choice of map or row
 *
 */

#ifndef included__map_or_row
#define included__map_or_row

#include "header.h"
#include <stdio.h>

/* Read a row either from a map or from a matrix */
extern int read_row(int is_perm, FILE *inp, word *row,
                    u32 nob, u32 noc, u32 len,
                    u32 i, const char *m, const char *name);

extern int read_rows(int is_perm, FILE *inp, word **rows,
                     u32 nob, u32 noc, u32 len,
                     u32 nor, const char *m, const char *name);

#endif
