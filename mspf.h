/*
 * $Id: mspf.h,v 1.2 2004/08/28 19:58:00 jon Exp $
 *
 * Function to spin some vectors under multiple generators
 * using intermediate files in a temporary directory.
 *
 */

#ifndef included__mspf
#define included__mspf

extern unsigned int spinf(const char *in, const char *out, const char *dir,
                          unsigned int argc, const char * const args[],
                          const char *name);

#endif
