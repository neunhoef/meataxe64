/*
 * $Id: nsf.h,v 1.1 2002/01/18 21:52:23 jon Exp $
 *
 * Function to compute nullspace of a matrix, from file, using temporary files
 *
 */

#ifndef included__nsf
#define included__nsf

extern unsigned int nullspace(const char *m1, const char *m2,
                              const char *dir, const char *name);

#endif
