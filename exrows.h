/*
 * $Id: exrows.h,v 1.2 2001/10/03 23:57:32 jon Exp $
 *
 * Extended row manipulation for meataxe
 *
 */

#ifndef included__exrows
#define included__exrows

extern int ex_row_put(unsigned int row_num, unsigned int total_cols, unsigned int total_rows,
                      const char *dir, const char *names[],
                      unsigned int split_size, const unsigned int *row, FILE *outputs[]);

#endif
