/*
 * $Id: grease.h,v 1.1 2001/09/02 22:16:41 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#ifndef included__grease
#define included__grease

extern int grease(unsigned int nob, unsigned int nor1, unsigned int noc1,
                  unsigned int noc2, unsigned int prime, unsigned int *step);

extern unsigned int **grease_make_rows(unsigned int **rows2, unsigned int size,
                                       unsigned int prime, unsigned int nob,
                                       unsigned int col_index, unsigned int *grease_row_count);

extern unsigned int grease_get_elt(const unsigned int *row1, unsigned int i,
                                   unsigned int size, unsigned int prime,
                                   unsigned int nob);

#endif
