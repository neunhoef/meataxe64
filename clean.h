/*
 * $Id: clean.h,v 1.1 2001/11/07 22:35:27 jon Exp $
 *
 * Cleaning and echilisation for meataxe
 *
 */

#ifndef included__clean
#define included__clean

#include "rows.h"

extern void clean(unsigned int **m1, unsigned int d1,
                  unsigned int **m2, unsigned int d2,
                  unsigned int *d_out, int *map,
                  unsigned int grease_level, unsigned int prime,
                  unsigned int len,
                  unsigned int nob, unsigned int start);

extern void clean_init(row_opsp ops);

#endif
