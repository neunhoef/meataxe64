/*
 * $Id: sign.h,v 1.1 2002/10/12 17:40:49 jon Exp $
 *
 * Function compute the orthogonal group sign
 *
 */

#ifndef included__sign
#define included__sign

#include <stdio.h>

/* Return 0 if +, 1 on error and 255 if - */
extern int sign(const char *qform, const char *bform, const char *name);

#endif
