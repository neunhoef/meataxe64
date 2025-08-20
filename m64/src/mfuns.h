/*
         mfuns.h  -   matrix functions prototypes
         =======      J.G.Thackray 13.10.2017
*/

/* This file contains utility maths functions until better places can be found for them */

#ifndef MFUNS_H
#define MFUNS_H

typedef union header {
  struct named_header {
    uint64_t rnd1;
    uint64_t fdef;
    uint64_t nor;
    uint64_t noc;
    uint64_t rnd2;
  } named;
  uint64_t hdr[5];
} header;

/*
 * Flatten into a row something specified as some zeroes,
 * then some -1s and then a remnant.
 * Used by theings like invariant subspace and subspace action
 */

extern void make_plain(const char *zero_bs, const char *nref_bs, const char *in, const char *out, uint64_t fdef);

extern int ident(uint64_t fdef, uint64_t nor, uint64_t noc, uint64_t elt,
                 const char *out);

/*
 * Function to do triaged multiply
 * Two bit strings and a remnant form the left hand argument
 * The first bitstring represnts the columns that are zero
 * The second acts only on the non zero columns, and splits them
 * into negative identity and a remnant.
 * If the first bitstring is NULL,
 * the second is assumed to cover the entire width
 * These are applied to what we wish to multiply by
 * to reduce the size of the multiply in terms of rows
 * tmp_vars is used for temporary files; the names must be allocated
 * and there must be at least 4 of them in the array
 * fun_tmp is passed through as a temporary file root
 */
extern void triage_multiply(const char *zbs, const char *sbs,
                            const char *rem, const char *in, const char *out,
                            const char *tmp_vars[], const char *fun_tmp);

/*
 * Slicing, splicing and chopping
 * slice and splice are used to reduce multiply to in memory capable
 * Chopping is used for recursive Gaussian elimination
 */
extern void slice(const char *input, unsigned int slices, const char *output_stem);

extern void splice(const char *input_stem, unsigned int slices, const char *output);

/*
 * Chop input into a chops by chops square collection of submatrices
 * whose names are given in the array outputs
 * outputs must be an array of size chops^2
 * The fragments are written out with columns varying fastest
 * mode says whether to log the input (output never logged)
 */
extern void chop(const char *input, unsigned int chops, const char *outputs[],
                 int mode);

/* Concatenate a number of files giving out */
extern void cat(const char *files[], const char *out, unsigned int count);

/* Invert the permutation found in m and return the result in memory */
extern uint64_t *perm_inv(const char *perm, uint64_t *nor);

/* Write out a bitstring to a file
 * Just the string itself is given, the header will be formed for nor and noc
 */

extern void fWriteBitString(const char *file, Dfmt *string, uint64_t nor, uint64_t noc);

#endif /* MFUNS_H */
