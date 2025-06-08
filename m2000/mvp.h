/*
 * $Id: mvp.h,v 1.1 2006/07/19 21:26:00 jon Exp $
 *
 * Function to permute some vectors under two generators
 *
 */

#ifndef included__mvp
#define included__mvp

/* Projective indicates whether to permuate points, or lines */
/* 0 => points, 1 => lines */
extern u32 multi_permute(const char *in, const char *out,
                         u32 argc, const char *const args[],
                         int projective, const char *name);

#endif
