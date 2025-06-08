/*
 * $Id: mspf.h,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Function to spin some vectors under multiple generators
 * using intermediate files in a temporary directory.
 *
 */

#ifndef included__mspf
#define included__mspf

extern u32 spinf(const char *in, const char *out, const char *dir,
                 unsigned int argc, const char * const args[],
                 const char *name);

#endif
