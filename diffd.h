/*
 * $Id: diffd.h,v 1.2 2005/06/22 21:52:53 jon Exp $
 *
 * Function to find the differences between the diagonal of a matrix and a scalar
 *
 */

#ifndef included__diffd
#define included__diffd

/* Only a return code is delivered */
/* 0 => failed, non-zero =: ok */
extern int diffd(const char *m, u32 elt, const char *name);

#endif
