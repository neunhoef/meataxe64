/*
 * $Id: sign.h,v 1.2 2005/06/22 21:52:54 jon Exp $
 *
 * Function compute the orthogonal group sign
 *
 */

#ifndef included__sign
#define included__sign

/* Return 0 if +, 1 on error and 255 if - */
extern int sign(const char *qform, const char *bform, const char *name);

#endif
