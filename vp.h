/*
 * $Id: vp.h,v 1.2 2002/06/25 10:30:12 jon Exp $
 *
 * Function to permute some vectors under two generators
 *
 */

#ifndef included__vp
#define included__vp

extern unsigned int permute(const char *in, const char *out, const char *a,
                            const char *b, const char *a_out,
                            const char *b_out, const char *name);

#endif
