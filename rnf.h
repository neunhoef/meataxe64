/*
 * $Id: rnf.h,v 1.2 2002/01/22 08:40:24 jon Exp $
 *
 * Function to compute row rank of a matrix, from file, using temporary files
 *
 */

#ifndef included__rnf
#define included__rnf

extern unsigned int rank(const char *m1, const char *dir, const char *m2,
                         int record, const char *name);

#endif
