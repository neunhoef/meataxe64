/*
 * $Id: powers.h,v 1.3 2002/02/27 19:06:17 jon Exp $
 *
 * Function to compute tensor powers of a matrix, from file
 *
 */

#ifndef included__powers
#define included__powers

extern int skew_square(const char *m1, const char *m2, const char *name);

extern int sym_square(const char *m1, const char *m2, const char *name);

extern int skew_cube(const char *m1, const char *m2, const char *name);

extern int skew_fourth(const char *m1, const char *m2, const char *name);

extern int skew_fifth(const char *m1, const char *m2, const char *name);

extern int skew_sixth(const char *m1, const char *m2, const char *name);

extern int skew_seventh(const char *m1, const char *m2, const char *name);

#endif
