/*
 * $Id: grease.h,v 1.7 2001/11/18 16:43:45 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#ifndef included__grease
#define included__grease

#include "rows.h"

typedef struct grease_struct
{
  unsigned int level;
  unsigned int size;
  unsigned int **rows;
  unsigned int *status; /* 1 => set, 0 => unset */
  row_ops row_operations;
} grease_struct, *grease;

/* Compute highest grease level given available space */
extern int grease_level(unsigned int prime,
                        grease grease, unsigned int avail);

/* Set up row operations */
extern void grease_init(row_opsp ops, grease grease);

/* Allocate the row table and row type table */
extern int grease_allocate(unsigned int prime, unsigned int len,
                           grease grease, unsigned int start);

/* Mark which rows are initial and which derived */
extern void grease_init_rows(grease grease, unsigned int prime);

/* Free the row table and row type table */
extern void grease_free(grease grease);

/* Compute the derived rows from the nitial rows */
extern int grease_make_rows(grease grease, unsigned int level,
                            unsigned int prime, unsigned int len);

#endif
