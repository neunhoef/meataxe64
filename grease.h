/*
 * $Id: grease.h,v 1.2 2001/09/04 23:00:12 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#ifndef included__grease
#define included__grease

extern int grease(unsigned int nob, unsigned int nor1, unsigned int noc1,
                  unsigned int noc2, unsigned int prime, unsigned int *step);

extern int grease_make_rows(unsigned int **rows_in, unsigned int size,
                            unsigned int prime, unsigned int noc,
                            unsigned int col_index,
                            unsigned int ***rows_out);

extern int grease_allocate_rows(unsigned int size,
                                unsigned int prime, unsigned int nob, unsigned int noc,
                                unsigned int *grease_row_count,
                                unsigned int ***rows_out);

#endif
