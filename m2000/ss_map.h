/*
 * $Id: ss_map.h,v 1.2 2005/06/22 21:52:54 jon Exp $
 *
 * Function to compute subspace map
 *
 */

#ifndef included__ss_map
#define included__ss_map

#include <stdio.h>

/* Given the range, the row length and field width */
/* and a pre-allcoated map for the results, and a row into which to read */
/* return 1 if ok, 0 for failure */
extern int subspace_map(FILE *inp, int *map, u32 nor,
                        u32 len, u32 nob,
                        word *row, const char *in,
                        const char *name);

#endif
