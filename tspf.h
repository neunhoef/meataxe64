/*
 * $Id: tspf.h,v 1.2 2004/08/28 19:58:01 jon Exp $
 *
 * Function to spin some vectors under two generators in tensor space
 * using intermediate files in a temporary directory.
 *
 */

#ifndef included__tspf
#define included__tspf

extern unsigned int tensor_spinf(const char *in, const char *out,
                                 const char *a1, const char *a2,
                                 const char *b1, const char *b2,
                                 const char *dir, const char *name);

#endif
