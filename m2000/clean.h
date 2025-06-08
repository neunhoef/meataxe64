/*
 * $Id: clean.h,v 1.12 2017/04/15 17:15:04 jon Exp $
 *
 * Cleaning and echilisation for meataxe
 *
 */

#ifndef included__clean
#define included__clean

#include "rows.h"

/* Clean m2 with m1 */
/* Record in m2_e with m1_e if record != 0 */
extern void clean(row_ops *row_operations,
                  word **m1, u32 d1,
                  word **m2, u32 d2, int *map,
                  word **m1_e, word **m2_e, int record,
                  u32 grease_level, u32 prime,
                  u32 len, u32 nob,
                  u32 start, u32 start_e,
                  u32 len_e, int verbose, const char *name);

/* Clean m2 with m1, specially for Z8 */
/* Record in m2_e with m1_e if record != 0 */
extern void clean2(row_ops *row_operations,
                   word **m1, u32 d1,
                   word **m2, u32 d2, int *map,
                   word **m1_e, word **m2_e, int record,
                   u32 grease_level, u32 prime,
                   u32 len, u32 nob,
                   u32 start, u32 start_e,
                   u32 len_e, int verbose, const char *name);

extern void echelise(row_ops *row_operations,
                     word **m, u32 d,
                     u32 *d_out, int **map,
                     word **m_e, int record,
                     u32 grease_level, u32 prime,
                     u32 len, u32 nob,
                     u32 start, u32 start_e,
                     u32 len_e,
                     int full, const char *name);
/*
 * Echelise, returning product of the pivots and a map of the pivot columns
 * This is so a determinant can be computed
 */
extern void echelise_with_det(row_ops *row_operations,
                              word **m, u32 d,
                              u32 *d_out, int **map,
                              word *det,
                              word **m_e, int record,
                              u32 grease_level, u32 prime,
                              u32 len, u32 nob,
                              u32 start, u32 start_e,
                              u32 len_e,
                              int full, const char *name);

/*
 * Echelise over Z/8, returning product of the pivots and a map of the pivot columns
 * This is so a determinant in GF(2) can be computed
 */
extern void echelise_with_det2(row_ops *row_operations,
                               word **m, u32 d,
                               u32 *d_out, int **map,
                               word *det,
                               word **m_e, int record,
                               u32 grease_level, u32 prime,
                               u32 len, u32 nob,
                               u32 start, u32 start_e,
                               u32 len_e,
                               int full, const char *name);

extern u32 simple_echelise(word **m, u32 d,
                           u32 prime,
                           u32 len, u32 nob);

extern u32 simple_echelise_and_record(word **m1, word **m2,
                                      u32 d, u32 prime,
                                      u32 len, u32 nob);

#endif
