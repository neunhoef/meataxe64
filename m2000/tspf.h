/*
 * $Id: tspf.h,v 1.3 2005/06/22 21:52:54 jon Exp $
 *
 * Function to spin some vectors under two generators in tensor space
 * using intermediate files in a temporary directory.
 *
 */

#ifndef included__tspf
#define included__tspf

extern u32 tensor_spinf(const char *in, const char *out,
                        const char *a1, const char *a2,
                        const char *b1, const char *b2,
                        const char *dir, const char *name);

#endif
