/*
 * $Id: grease.h,v 1.6 2001/11/07 22:35:27 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#ifndef included__grease
#define included__grease

#include "rows.h"

/* Compute highest grease level given available space */
extern int grease(unsigned int prime,
                  unsigned int *step, unsigned int avail);

/* Compute the derived rows from the nitial rows */
extern int grease_make_rows(unsigned int size,
                            unsigned int prime, unsigned int len);

/* Mark which rows are initial and which derived */
extern void grease_init_rows(unsigned int size, unsigned int prime);

/* Allocate the row table and row type table */
extern int grease_allocate_rows(unsigned int size,
                                unsigned int prime, unsigned int len,
                                unsigned int ***rows_out, unsigned int start);

/* Free the row table and row type table */
extern void grease_free_rows(unsigned int **rows);

/* Set up row operations */
extern void grease_init(row_opsp ops);

#endif
