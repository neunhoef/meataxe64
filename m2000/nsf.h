/*
 * $Id: nsf.h,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Function to compute nullspace of a matrix, from file, using temporary files
 *
 */

#ifndef included__nsf
#define included__nsf

extern u32 nullspacef(const char *m1, const char *m2,
                      const char *dir, const char *name);

#endif
