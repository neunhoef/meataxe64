/*
 * $Id: exrows.h,v 1.4 2005/06/22 21:52:53 jon Exp $
 *
 * Extended row manipulation for meataxe
 *
 */

#ifndef included__exrows
#define included__exrows

#include "header.h"

extern int ex_row_put(u32 row_num, u32 total_cols, u32 total_rows,
                      const char *dir, const char *names[],
                      u32 split_size, const word *row, FILE *outputs[]);

extern int ex_row_get(u32 col_pieces, FILE **inputs, const header **headers,
                      word *row1, word *row2, const char *name, const char **names,
                      u32 i, u32 nob);

#endif
