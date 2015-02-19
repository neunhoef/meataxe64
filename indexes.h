/*
 * Compute indexes into rows, telling us how much can be ignored
 * because it's zero
 */

#ifndef included__indexes
#define included__indexes

#include <stdio.h>

/*
 * 0 return => failed
 */
extern int make_indexes(FILE *f, word *row, u32 *indexes, u32 nor, u32 len, const char *name, const char *m);

#endif
