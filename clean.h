/*
 * $Id: clean.h,v 1.4 2001/11/17 10:02:43 jon Exp $
 *
 * Cleaning and echilisation for meataxe
 *
 */

#ifndef included__clean
#define included__clean

#include "rows.h"

/* Clean m2 with m1 */
extern void clean(unsigned int **m1, unsigned int d1,
                  unsigned int **m2, unsigned int d2,
                  unsigned int *d_out, int *map,
                  unsigned int grease_level, unsigned int prime,
                  unsigned int len, unsigned int nob,
                  unsigned int start, const char *name);

extern void echelise(unsigned int **m, unsigned int d,
                     unsigned int *d_out, int **map,
                     unsigned int ***m_out,
                     unsigned int grease_level, unsigned int prime,
                     unsigned int len, unsigned int nob,
                     unsigned int start, const char *name);

extern unsigned int simple_echelise(unsigned int **m, unsigned int d,
                                    unsigned int prime,
                                    unsigned int len, unsigned int nob);

extern unsigned int simple_echelise_and_record(unsigned int **m1, unsigned int **m2,
                                               unsigned int d, unsigned int prime,
                                               unsigned int len, unsigned int nob);

extern void clean_init(row_opsp ops);

#endif
