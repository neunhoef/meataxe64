/*
 * $Id: mul.h,v 1.4 2002/04/10 23:33:27 jon Exp $
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

/* Multiply rows1 by m giving rows3. m may be a map, if is_map is non-zero */
extern int mul_from_store(unsigned int **rows1, unsigned int **rows3,
                          FILE *inp, int is_map, unsigned int noc, unsigned int len,
                          unsigned int nob, unsigned int nor, unsigned int noc_o, unsigned int prime,
                          grease grease, const char *m, const char *name);


#endif
