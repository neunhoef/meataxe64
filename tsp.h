/*
 * $Id: tsp.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Function to spin some vectors under two generators in tensor space
 *
 */

#ifndef included__tsp
#define included__tsp

extern unsigned int spin(const char *in, const char *out,
                         const char *a1, const char *a2,
                         const char *b1, const char *b2, const char *name);

#endif
