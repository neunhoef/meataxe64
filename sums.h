/*
 * $Id: sums.h,v 1.7 2003/08/10 14:30:25 jon Exp $
 *
 * Function to compute linear sums of two matices
 *
 */

#ifndef included__sums
#define included__sums

#include "sums_utils.h"

/* Return 255 for failed to find an element */
/* Return 0 for success */
/* Return 1 for parameter error */
extern int sums(const char *out, unsigned int n, unsigned int argc, const char *const args[],
                unsigned int sub_order, accept acceptor, int invertible, int keep, const char *name);

#endif
