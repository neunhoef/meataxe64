/*
 * $Id: grease.h,v 1.4 2001/09/18 23:15:46 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#ifndef included__grease
#define included__grease

#include "rows.h"

extern int grease(unsigned int nob, unsigned int prime,
                  unsigned int *step, unsigned int avail);

extern int grease_make_rows(unsigned int size,
                            unsigned int prime, unsigned int len);

extern int grease_allocate_rows(unsigned int size,
                                unsigned int prime, unsigned int len,
                                unsigned int ***rows_out, unsigned int start);

extern void grease_free_rows(unsigned int **rows);

extern void grease_init(row_opsp ops);

#endif
