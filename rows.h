/*
 * $Id: rows.h,v 1.14 2004/06/12 16:54:27 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#ifndef included__rows
#define included__rows

extern void row_copy(const unsigned int *, unsigned int *,
                     unsigned int);

extern void row_init(unsigned int *, unsigned int len);

extern int row_is_zero(unsigned int *, unsigned int len);

typedef void (*row_adder)(const unsigned int *, const unsigned int *,
                          unsigned int *, unsigned int);

typedef void (*row_incer)(const unsigned int *,
                          unsigned int *, unsigned int);

typedef void (*scaled_row_adder)(const unsigned int *, const unsigned int *,
                                 unsigned int *, unsigned int, unsigned int);

typedef void (*scaled_row_incer)(const unsigned int *, unsigned int *,
                                 unsigned int, unsigned int);

typedef void (*row_scaler)(const unsigned int *, unsigned int *,
                           unsigned int, unsigned int);

typedef void (*row_scaler_in_place)(unsigned int *,
                                    unsigned int, unsigned int);

typedef unsigned int (*row_inner_product)(const unsigned int *, const unsigned int *, unsigned int);

typedef struct {
  row_adder adder;
  row_incer incer;
  scaled_row_adder scaled_adder;
  scaled_row_incer scaled_incer;
  row_scaler scaler;
  row_scaler_in_place scaler_in_place;
  row_inner_product product;
} row_ops, *row_opsp;

extern int rows_init(unsigned int prime, row_opsp ops);

extern int short_rows_init(unsigned int prime, row_opsp ops);

#endif
