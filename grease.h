/*
 * $Id: grease.h,v 1.12 2004/06/06 09:38:56 jon Exp $
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
  unsigned int *quot;
  unsigned int *rem;
  unsigned int *index;
  row_ops row_operations;
} grease_struct, *grease;

/* Compute highest grease level given available space */
extern int grease_level(unsigned int prime, grease grease, unsigned int avail);

/* Compute a sensible grease level given available number of rows */
extern int grease_limit(unsigned int prime, unsigned int level, unsigned int grease_rows, unsigned int total_rows);

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
/* The row is assumed zero prior to offset (which is in words) */
extern void grease_row_add(grease grease, unsigned int len, unsigned int *row1,
                           const unsigned int *row2, unsigned int element,
                           unsigned int offset);

/* row1 += grease_row[element], creating necessary grease rows en route */
/* The row is assumed zero prior to offset (which is in words) */
extern void grease_row_inc(grease grease, unsigned int len, unsigned int *row,
                           unsigned int element, unsigned int offset);

#endif
