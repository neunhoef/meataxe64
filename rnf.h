/*
 * $Id: rnf.h,v 1.3 2002/02/21 20:37:21 jon Exp $
 *
 * Function to compute row rank of a matrix, from file, using temporary files
 *
 */

#ifndef included__rnf
#define included__rnf

extern unsigned int rank(const char *m1, const char *dir, const char *m2,
                         int record, int full, const char *name);

#endif
