/*
 * $Id: sumsf.h,v 1.7 2003/08/10 14:30:25 jon Exp $
 *
 * Function to compute linear sums of two matices, using intermediate files
 *
 */

#ifndef included__sumsf
#define included__sumsf

#include "sums_utils.h"

/* Return 255 for failed to find an element */
/* Return 0 for success */
/* Return 1 for parameter error */
extern int sumsf(const char *out, const char *dir, unsigned int n, unsigned int argc, const char *const args[],
                 unsigned int sub_order, accept acceptor, int invertible, int keep, const char *name);

#endif
