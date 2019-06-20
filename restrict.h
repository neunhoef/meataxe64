/*
 * $Id: restrict.h,v 1.3 2019/06/20 22:50:28 jon Exp $
 *
 * Function to restrict a matrix from a big field to a smaller
 *
 */

#ifndef included__restrict
#define included__restrict

extern int res_restrict(const char *m1, const char *m2, u32 q, const char *name);

#endif
