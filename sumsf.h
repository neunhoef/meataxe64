/*
 * $Id: sumsf.h,v 1.3 2002/09/11 10:02:28 jon Exp $
 *
 * Function to compute linear sums of two matices, using intermediate files
 *
 */

#ifndef included__sumsf
#define included__sumsf

typedef int (*accept)(unsigned int rank, unsigned int nor, const char *file, const char *form);

extern int sumsf(const char *in1, const char *in2, const char *out,
                 const char *dir,
                 unsigned int o_a, unsigned o_b, unsigned int n,
                 unsigned int sub_order, accept acceptor, const char *name);

#endif
