/*
 * $Id: rnf.h,v 1.4 2002/09/16 10:24:07 jon Exp $
 *
 * Function to compute row rank of a matrix, from file, using temporary files
 *
 */

#ifndef included__rnf
#define included__rnf

extern unsigned int rank(const char *m1, const char *dir, const char *name);

#endif
