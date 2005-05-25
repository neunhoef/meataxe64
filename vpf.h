/*
 * $Id: vpf.h,v 1.1 2005/05/25 18:35:56 jon Exp $
 *
 * Function to permute some vectors under two generators,
 * using intermediate file
 */

#ifndef included__vpf
#define included__vpf

/* Projective indicates whether to permuate points, or lines */
/* 0 => points, 1 => lines */
extern unsigned int permute_file(const char *tmp_dir, const char *in,
                                 const char *out, const char *a,
                                 const char *b, const char *a_out,
                                 const char *b_out,
                                 int projective, const char *name);

#endif
