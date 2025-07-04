/*
 * $Id: tsp.h,v 1.3 2005/06/22 21:52:54 jon Exp $
 *
 * Function to spin some vectors under two generators in tensor space
 *
 */

#ifndef included__tsp
#define included__tsp

extern u32 tensor_spin(const char *in, const char *out,
                       const char *a1, const char *a2,
                       const char *b1, const char *b2, const char *name);

#endif
