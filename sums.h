/*
 * $Id: sums.h,v 1.2 2002/03/24 19:44:02 jon Exp $
 *
 * Function to compute linear sums of two matices
 *
 */

#ifndef included__sums
#define included__sums

typedef int (*accept)(unsigned int rank, unsigned int nor);

extern int sums(const char *in1, const char *in2, const char *out,
                unsigned int o_a, unsigned o_b, unsigned int n,
                unsigned int sub_order,
                const char *name, accept acceptor);

#endif
