/*
 * $Id: mul.h,v 1.11 2005/06/22 21:52:53 jon Exp $
 *
 * Function to multiply two matrices to give a third
 *
 */

#ifndef included__mul
#define included__mul

#include <stdio.h>
#include "grease.h"

/* Multiply m1 by m2 giving m3. Either of m1 and m2 may be a map */
/* If both are maps, then m3 will be a map */
extern int mul(const char *m1, const char *m2, const char *m3, const char *name);

/* Multiply rows1 by m giving rows3. m is a map, if is_map is non-zero */
extern int mul_from_store(word **rows1, word **rows3,
                          FILE *inp, int is_map, u32 noc_i, u32 len_o,
                          u32 nob, u32 nor_i, u32 noc_o, u32 prime,
                          grease grease, int verbose, const char *m, const char *name);

/* Multiply rows1 by m, starting at offset, giving rows3. m is a map, if is_map is non-zero */
extern int skip_mul_from_store(u32 offset, word **rows1, word **rows3,
                          FILE *inp, int is_map, u32 noc_i, u32 len_o,
                          u32 nob, u32 nor_i, u32 noc_o, u32 prime,
                          grease grease, int verbose, const char *m, const char *name);

/* Entirely in store multiply */
/* Either of rows1, rows2 may be a map. If both are, then so will rows3 */
/* If at least one of m1, m2 is not a map, then len is output length */
extern int mul_in_store(word **rows1, word **rows2, word **rows3,
                        u32 noc_i, u32 len_o,
                        u32 nob, u32 nor_i, u32 prime,
                        int preserve_rows,
                        grease grease);


#endif
