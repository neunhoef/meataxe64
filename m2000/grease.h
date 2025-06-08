/*
 * $Id: grease.h,v 1.14 2018/04/28 16:22:42 jon Exp $
 *
 * Functions to grease matrix rows
 *
 */

#ifndef included__grease
#define included__grease

#include "rows.h"

typedef struct grease_struct
{
  u32 level;
  word size;
  word **rows;
  u32 *status; /* 1 => set, 0 => unset */
  u32 *quot;
  u32 *rem;
  u32 *index;
  row_ops row_operations;
} grease_struct, *grease;

/* Compute highest grease level given available space */
extern int grease_level(u32 prime, grease grease, u32 avail);

/* Compute a sensible grease level given available number of rows */
extern int grease_limit(u32 prime, u32 level, u32 grease_rows, u32 total_rows);

/* Set up row operations */
extern void grease_init(row_opsp ops, grease grease);

/* Allocate and populate the divisions table */
extern int grease_allocate_table(u32 prime, grease grease);

/* Allocate the row table and row type table */
extern int grease_allocate(u32 prime, u32 len,
                           grease grease, u32 start);

/* Mark which rows are initial and which derived */
extern void grease_init_rows(grease grease, u32 prime);

/* Free the row table and row type table */
extern void grease_free(grease grease);

/* row1 = row2 + grease_row[element], creating necessary grease rows en route */
/* The row is assumed zero prior to offset (which is in words) */
extern void grease_row_add(grease grease, u32 len, word *row1,
                           const word *row2, u32 element,
                           u32 offset);

/* row1 += grease_row[element], creating necessary grease rows en route */
/* The row is assumed zero prior to offset (which is in words) */
extern void grease_row_inc(grease grease, u32 len, word *row,
                           u32 element, u32 offset);

#endif
