/*
 * $Id: ttr.h,v 1.1 2003/06/21 14:04:24 jon Exp $
 *
 * Function to transpose some tensor product vectors
 *
 */

#ifndef included__ttr
#define included__ttr

/* Transpose every vector in m1, regarded as a tensor with input_noc rows */
extern int ttr(unsigned int input_noc, const char *m1, const char *m2, const char *name);

#endif
