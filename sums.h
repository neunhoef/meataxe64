/*
 * $Id: sums.h,v 1.5 2002/09/24 19:08:54 jon Exp $
 *
 * Function to compute linear sums of two matices
 *
 */

#ifndef included__sums
#define included__sums

typedef int (*accept)(unsigned int rank, unsigned int nor, const char *file, const char *form);

/* Return 255 for failed to find an element */
/* Return 0 for success */
/* Return 1 for parameter error */
extern int sums(const char *out, unsigned int n, unsigned int argc, const char *const args[],
                unsigned int sub_order, accept acceptor, const char *name);

#endif
