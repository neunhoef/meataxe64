/*
 * $Id: sums.h,v 1.6 2003/08/04 20:41:57 jon Exp $
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
                unsigned int sub_order, accept acceptor, int invertible, const char *name);

#endif
