/*
 * $Id: rows.h,v 1.7 2001/09/20 00:00:16 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#ifndef included__rows
#define included__rows

extern void row_copy(const unsigned int *, unsigned int *,
                     unsigned int);

extern void row_init(unsigned int *, unsigned int len);

typedef int (*row_adder)(const unsigned int *, const unsigned int *,
                         unsigned int *, unsigned int);

typedef int (*row_incer)(const unsigned int *,
                         unsigned int *, unsigned int);

typedef int (*scaled_row_adder)(const unsigned int *, const unsigned int *,
                                unsigned int *, unsigned int, unsigned int);

typedef int (*row_scaler)(const unsigned int *, unsigned int *,
                          unsigned int, unsigned int);

typedef struct {
  row_adder adder;
  row_incer incer;
  scaled_row_adder scaled_adder;
  row_scaler scaler;
} row_ops, *row_opsp;

extern int rows_init(unsigned int prime, row_opsp ops);

#endif
