/*
 * $Id: rnf.h,v 1.5 2004/08/28 19:58:00 jon Exp $
 *
 * Function to compute row rank of a matrix, from file, using temporary files
 *
 */

#ifndef included__rnf
#define included__rnf

extern unsigned int rankf(const char *m1, const char *dir, const char *name);

#endif
