/*
 * $Id: tmul.h,v 1.1 2002/06/25 10:30:12 jon Exp $
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

#endif
