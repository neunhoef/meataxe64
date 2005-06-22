/*
 * $Id: ttr.h,v 1.2 2005/06/22 21:52:54 jon Exp $
 *
 * Function to transpose some tensor product vectors
 *
 */

#ifndef included__ttr
#define included__ttr

/* Transpose every vector in m1, regarded as a tensor with input_noc rows */
extern int ttr(u32 input_noc, const char *m1, const char *m2, const char *name);

#endif
