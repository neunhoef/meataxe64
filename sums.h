/*
 * $Id: sums.h,v 1.4 2002/09/11 10:02:28 jon Exp $
 *
 * Function to compute linear sums of two matices
 *
 */

#ifndef included__sums
#define included__sums

typedef int (*accept)(unsigned int rank, unsigned int nor, const char *file, const char *form);

extern int sums(const char *out, unsigned int n, unsigned int argc, const char *const args[],
                unsigned int sub_order, accept acceptor, const char *name);

#endif
