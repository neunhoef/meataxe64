/*
 * $Id: powers.h,v 1.2 2002/02/12 23:10:24 jon Exp $
 *
 * Function to compute tensor powers of a matrix, from file
 *
 */

#ifndef included__powers
#define included__powers

extern int skew_square(const char *m1, const char *m2, const char *name);

extern int sym_square(const char *m1, const char *m2, const char *name);

extern int skew_cube(const char *m1, const char *m2, const char *name);

#endif
