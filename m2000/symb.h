/*
 * $Id%
 *
 * Function to compute a symmetry basis.
 * Returns the total dimension
 *
 */

#ifndef included__symb
#define included__symb

extern u32 symb(u32 spaces, u32 space_size,
                const char *in, const char *out, const char *dir,
                unsigned int argc, const char * const args[],
                const char *name);

#endif
