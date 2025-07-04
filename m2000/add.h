/*
 * $Id: add.h,v 1.5 2005/06/22 21:52:53 jon Exp $
 *
 * Function to add two matrices to give a third
 *
 */

#ifndef included__add
#define included__add

extern int add(const char *m1, const char *m2, const char *m3, const char *name);

/* Generate lambda * m1 + m2 */
extern int scaled_add(const char *m1, const char *m2, const char *m3, u32 scalar, const char *name);

#endif
