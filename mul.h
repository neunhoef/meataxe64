/*
 * $Id: mul.h,v 1.8 2004/02/15 10:27:17 jon Exp $
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
extern int mul_from_store(unsigned int **rows1, unsigned int **rows3,
                          FILE *inp, int is_map, unsigned int noc_i, unsigned int len_o,
                          unsigned int nob, unsigned int nor_i, unsigned int noc_o, unsigned int prime,
                          grease grease, int verbose, const char *m, const char *name);

/* Multiply rows1 by m, starting at offset, giving rows3. m is a map, if is_map is non-zero */
extern int skip_mul_from_store(unsigned int offset, unsigned int **rows1, unsigned int **rows3,
                          FILE *inp, int is_map, unsigned int noc_i, unsigned int len_o,
                          unsigned int nob, unsigned int nor_i, unsigned int noc_o, unsigned int prime,
                          grease grease, int verbose, const char *m, const char *name);

/* Entirely in store multiply */
/* Either of rows1, rows2 may be a map. If both are, then so will rows3 */
/* If at least one of m1, m2 is not a map, then len is output length */
extern int mul_in_store(unsigned int **rows1, unsigned int **rows2, unsigned int **rows3,
                        int is_map1, int is_map2, unsigned int noc_i, unsigned int len_o,
                        unsigned int nob, unsigned int nor_i, unsigned int noc_o, unsigned int prime,
                        grease grease, const char *m1, const char *m2, const char *name);


#endif
