/*
 * $Id: diffd.h,v 1.1 2003/01/02 20:37:40 jon Exp $
 *
 * Function to find the differences between the diagonal of a matrix and a scalar
 *
 */

#ifndef included__diffd
#define included__diffd

/* Only a return code is delivered */
/* 0 => failed, non-zero =: ok */
extern int diffd(const char *m, unsigned int elt, const char *name);

#endif
