/*
 * $Id%
 *
 * Function to compute a symmetry basis.
 * Returns the total dimension
 *
 */

#ifndef included__symb
#define included__symb

extern unsigned int symb(unsigned int spaces, unsigned int space_size,
                         const char *in, const char *out, const char *dir,
                         unsigned int argc, const char * const args[],
                         const char *name);

#endif
