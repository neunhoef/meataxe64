/*
 * $Id: map_or_row.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Handle reading from a choice of map or row
 *
 */

#ifndef included__map_or_row
#define included__map_or_row

#include "header.h"
#include <stdio.h>

/* Read a row either from a map or from a matrix */
extern int read_row(int is_perm, FILE *inp, unsigned int *row,
                    unsigned int nob, unsigned int noc, unsigned int len,
                    unsigned int i, const char *m, const char *name);

extern int read_rows(int is_perm, FILE *inp, unsigned int **rows,
                     unsigned int nob, unsigned int noc, unsigned int len,
                     unsigned int nor, const char *m, const char *name);

#endif
