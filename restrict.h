/*
 * $Id: restrict.h,v 1.1 2002/03/20 18:42:30 jon Exp $
 *
 * Function to restrict a matrix from a big field to a smaller
 *
 */

#ifndef included__restrict
#define included__restrict

extern int restrict(const char *m1, const char *m2, unsigned int q, const char *name);

#endif
