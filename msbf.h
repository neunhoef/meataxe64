/*
 * $Id: msbf.h,v 1.1 2002/07/07 12:10:42 jon Exp $
 *
 * Function to spin some vectors under multiple generators to obtain a standard base
 * using intermediate files in a temporary directory.
 *
 */

#ifndef included__msbf
#define included__msbf

extern unsigned int spin(const char *in, const char *out, const char *dir,
                         unsigned int argc, const char * const args[],
                         const char *name);

#endif
