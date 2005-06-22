/*
 * $Id: restrict.h,v 1.2 2005/06/22 21:52:53 jon Exp $
 *
 * Function to restrict a matrix from a big field to a smaller
 *
 */

#ifndef included__restrict
#define included__restrict

extern int restrict(const char *m1, const char *m2, u32 q, const char *name);

#endif
