/*
 * $Id: rows.h,v 1.4 2001/09/08 12:40:55 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#ifndef included__rows
#define included__rows

extern void row_copy(const unsigned int *, unsigned int *,
                     unsigned int, unsigned int);

extern int row_malloc(unsigned int nob, unsigned int noc, unsigned int **row);

extern void row_init(unsigned int *, unsigned int nob, unsigned int noc);

typedef int (*row_adder)(const unsigned int *, const unsigned int *,
                         unsigned int *, unsigned int);

typedef int (*scaled_row_adder)(const unsigned int *, const unsigned int *,
                                unsigned int *, unsigned int, unsigned int);

typedef struct {
  row_adder adder;
  scaled_row_adder scaled_adder;
} row_ops, *row_opsp;

extern int rows_init(unsigned int prime, row_opsp ops);

#endif
