/*
 * $Id: add.h,v 1.3 2002/01/06 16:35:48 jon Exp $
 *
 * Function to add two matrices to give a third
 *
 */

#ifndef included__add
#define included__add

extern int add(const char *m1, const char *m2, const char *m3, const char *name);

extern int scaled_add(const char *m1, const char *m2, const char *m3, unsigned int scalar, const char *name);

#endif
