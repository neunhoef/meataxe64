/*
 * $Id: grease.h,v 1.8 2001/12/23 23:31:42 jon Exp $
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

/* row1 = row2 + grease_row[element], creating necessary grease rows en route */
extern void grease_row_add(grease grease, unsigned int len, unsigned int *row1,
                           const unsigned int *row2, unsigned int element);

/* row1 += grease_row[element] , creating necessary grease rows en route */
extern void grease_row_inc(grease grease, unsigned int len, unsigned int *row,
                           unsigned int prime, unsigned int element);

#endif
