/*
 * $Id: rows.h,v 1.3 2001/09/04 23:00:12 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#ifndef included__rows
#define included__rows

extern int row_add(const unsigned int *, const unsigned int *,
                   unsigned int *, unsigned int prime,
                   unsigned int nob, unsigned int noc);

extern void row_copy(const unsigned int *, unsigned int *,
                     unsigned int, unsigned int);

extern int row_malloc(unsigned int nob, unsigned int noc, unsigned int **row);

extern void row_init(unsigned int *, unsigned int nob, unsigned int noc);

typedef int (*row_adder)(const unsigned int *, const unsigned int *,
                         unsigned int *, unsigned int);

typedef struct {
  row_adder adder;
} row_ops, *row_opsp;

extern int rows_init(unsigned int prime, row_opsp ops);

#endif
