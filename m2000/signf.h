/*
 * $Id: signf.h,v 1.1 2003/12/31 16:46:51 jon Exp $
 *
 * Function compute the orthogonal group sign
 *
 */

#ifndef included__sign
#define included__sign

#include <stdio.h>

/* Return 0 if +, 1 on error and 255 if - */
extern int sign(const char *qform, const char *bform, const char *dir, const char *name);

#endif
