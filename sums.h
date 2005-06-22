/*
 * $Id: sums.h,v 1.8 2005/06/22 21:52:54 jon Exp $
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
extern int sums(const char *out, u32 n, unsigned int argc, const char *const args[],
                u32 sub_order, accept acceptor, int invertible, int keep, const char *name);

#endif
