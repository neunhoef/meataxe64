/*
 * $Id: vp.h,v 1.3 2002/07/03 12:06:55 jon Exp $
 *
 * Function to permute some vectors under two generators
 *
 */

#ifndef included__vp
#define included__vp

/* Projective indicates whether to permuate points, or lines */
/* 0 => points, 1 => lines */
extern unsigned int permute(const char *in, const char *out, const char *a,
                            const char *b, const char *a_out, const char *b_out,
                            int projective, const char *name);

#endif
