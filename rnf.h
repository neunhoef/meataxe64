/*
 * $Id: rnf.h,v 1.1 2001/12/15 20:47:27 jon Exp $
 *
 * Function to compute row rank of a matrix, from file, using temporary files
 *
 */

#ifndef included__rnf
#define included__rnf

extern unsigned int rank(const char *m, const char *dir, const char *name);

#endif
