/*
 * $Id: rnf.h,v 1.6 2005/06/22 21:52:53 jon Exp $
 *
 * Function to compute row rank of a matrix, from file, using temporary files
 *
 */

#ifndef included__rnf
#define included__rnf

extern u32 rankf(const char *m1, const char *dir, const char *name);

#endif
