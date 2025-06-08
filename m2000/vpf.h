/*
 * $Id: vpf.h,v 1.2 2005/06/22 21:52:54 jon Exp $
 *
 * Function to permute some vectors under two generators,
 * using intermediate file
 */

#ifndef included__vpf
#define included__vpf

/* Projective indicates whether to permuate points, or lines */
/* 0 => points, 1 => lines */
extern u32 permute_file(const char *tmp_dir, const char *in,
                        const char *out, const char *a,
                        const char *b, const char *a_out,
                        const char *b_out,
                        int projective, const char *name);

#endif
