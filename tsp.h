/*
 * $Id: tsp.h,v 1.2 2004/08/28 19:58:01 jon Exp $
 *
 * Function to spin some vectors under two generators in tensor space
 *
 */

#ifndef included__tsp
#define included__tsp

extern unsigned int tensor_spin(const char *in, const char *out,
                         const char *a1, const char *a2,
                         const char *b1, const char *b2, const char *name);

#endif
