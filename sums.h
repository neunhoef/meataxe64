/*
 * $Id: sums.h,v 1.1 2002/02/18 20:42:49 jon Exp $
 *
 * Function to compute linear sums of two matices
 *
 */

#ifndef included__sums
#define included__sums

typedef int (*accept)(unsigned int rank, unsigned int nor);

extern int sums(const char *in1, const char *in2, const char *out,
                unsigned int o_a, unsigned o_b, unsigned int n,
                const char *name, accept acceptor);

#endif
