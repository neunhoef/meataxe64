/*
 * $Id: nsf.h,v 1.2 2004/08/28 19:58:00 jon Exp $
 *
 * Function to compute nullspace of a matrix, from file, using temporary files
 *
 */

#ifndef included__nsf
#define included__nsf

extern unsigned int nullspacef(const char *m1, const char *m2,
                               const char *dir, const char *name);

#endif
