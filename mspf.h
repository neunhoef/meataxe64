/*
 * $Id: mspf.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Function to spin some vectors under multiple generators
 * using intermediate files in a temporary directory.
 *
 */

#ifndef included__mspf
#define included__mspf

extern unsigned int spin(const char *in, const char *out, const char *dir,
                         unsigned int argc, const char * const args[],
                         const char *name);

#endif
