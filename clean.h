/*
 * $Id: clean.h,v 1.6 2001/11/19 18:31:48 jon Exp $
 *
 * Cleaning and echilisation for meataxe
 *
 */

#ifndef included__clean
#define included__clean

#include "rows.h"

/* Clean m2 with m1 */
/* Record in m2_e with m1_e if record != 0 */
extern void clean(unsigned int **m1, unsigned int d1,
                  unsigned int **m2, unsigned int d2, int *map,
                  unsigned int **m1_e, unsigned int **m2_e, int record,
                  unsigned int grease_level, unsigned int prime,
                  unsigned int len, unsigned int nob,
                  unsigned int start, unsigned int start_e,
                  unsigned int len_e, const char *name);

extern void echelise(unsigned int **m, unsigned int d,
                     unsigned int *d_out, int **map,
                     unsigned int **m_e, int record,
                     unsigned int grease_level, unsigned int prime,
                     unsigned int len, unsigned int nob,
                     unsigned int start, unsigned int start_e,
                     unsigned int len_e,
                     int full, const char *name);

extern unsigned int simple_echelise(unsigned int **m, unsigned int d,
                                    unsigned int prime,
                                    unsigned int len, unsigned int nob);

extern unsigned int simple_echelise_and_record(unsigned int **m1, unsigned int **m2,
                                               unsigned int d, unsigned int prime,
                                               unsigned int len, unsigned int nob);

#endif
