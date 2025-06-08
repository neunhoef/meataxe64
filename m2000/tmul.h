/*
 * $Id: tmul.h,v 1.2 2021/08/02 18:19:40 jon Exp $
 *
 * Function to multiply a matrix by a tensor product
 *
 */

#ifndef included__tmul
#define included__tmul

/* Multiply m1 by m2 tensor m3 giving m4. Either of m2 and m3 may be a map */
/* m1 may not be a map */
extern int tmul(const char *m1, const char *m2, const char *m3,
                const char *m4, const char *name);

/* Multiply a padded vector m1 treated as a tensor
 * Similar to tmul, except that it doesn't need to unpack
 * m1 before applying the tensors
 * nor(m1) is as before, but now noc(m1) = noc(m2) * padded(noc(m3))
 * where padded(noc) is the number of columns that would exist if
 * all of len were used
 */
extern int tmulp(const char *m1, const char *m2, const char *m3,
                 const char *m4, const char *name);

#endif
