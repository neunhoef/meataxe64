/*
 * $Id: sumsf.h,v 1.1 2002/05/26 00:47:20 jon Exp $
 *
 * Function to compute linear sums of two matices, using intermediate files
 *
 */

#ifndef included__sumsf
#define included__sumsf

typedef int (*accept)(unsigned int rank, unsigned int nor);

extern int sumsf(const char *in1, const char *in2, const char *out,
                 const char *dir,
                 unsigned int o_a, unsigned o_b, unsigned int n,
                 unsigned int sub_order,
                 const char *name, accept acceptor);

#endif
