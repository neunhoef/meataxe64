/*
 * $Id: rows.h,v 1.17 2015/02/09 23:33:13 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#ifndef included__rows
#define included__rows

extern void row_init(word *, u32 len);

extern int row_is_zero(const word *, u32 len);

typedef void (*row_adder)(const word *, const word *,
                          word *, u32);

typedef void (*row_incer)(const word *,
                          word *, u32);

typedef void (*scaled_row_adder)(const word *, const word *,
                                 word *, u32, word);

typedef void (*scaled_row_incer)(const word *, word *,
                                 u32, word);

typedef void (*row_scaler)(const word *, word *,
                           u32, word);

typedef void (*row_scaler_in_place)(word *,
                                    u32, word);

typedef word (*row_inner_product)(const word *, const word *, u32);

typedef struct {
  row_adder adder;
  row_incer incer;
  scaled_row_adder scaled_adder;
  scaled_row_incer scaled_incer;
  row_scaler scaler;
  row_scaler_in_place scaler_in_place;
  row_inner_product product;
} row_ops, *row_opsp;

extern int rows_init(u32 prime, row_opsp ops);

extern int short_rows_init(u32 prime, row_opsp ops);

extern int word_rows_init(u32 prime, row_opsp ops);

#endif
