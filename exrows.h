/*
 * $Id: exrows.h,v 1.1 2001/10/03 00:01:42 jon Exp $
 *
 * Extended row manipulation for meataxe
 *
 */

#ifndef included__exrows
#define included__exrows

extern int ex_row_put(unsigned int row_num, unsigned int total_cols,
                      const char *dir, const char *names[],
                      unsigned int split_size, const unsigned int *row, FILE *outputs[]);

#endif
