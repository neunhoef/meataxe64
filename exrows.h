/*
 * $Id: exrows.h,v 1.3 2001/12/27 01:17:12 jon Exp $
 *
 * Extended row manipulation for meataxe
 *
 */

#ifndef included__exrows
#define included__exrows

#include "header.h"

extern int ex_row_put(unsigned int row_num, unsigned int total_cols, unsigned int total_rows,
                      const char *dir, const char *names[],
                      unsigned int split_size, const unsigned int *row, FILE *outputs[]);

extern int ex_row_get(unsigned int col_pieces, FILE **inputs, const header **headers,
                      unsigned int *row1, unsigned int *row2, const char *name, const char **names,
                      unsigned int i, unsigned int nob);

#endif
