/*
 * $Id: ss_map.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Function to compute subspace map
 *
 */

#ifndef included__ss_map
#define included__ss_map

#include <stdio.h>

/* Given the range, the row length and field width */
/* and a pre-allcoated map for the results, and a row int which to read */
/* return 1 if ok, 0 for failure */
extern int subspace_map(FILE *inp, int *map, unsigned int nor,
                        unsigned int len, unsigned int nob,
                        unsigned int *row, const char *in,
                        const char *name);

#endif
