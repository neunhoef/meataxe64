/*
 * $Id: spf.h,v 1.1 2002/03/31 20:55:41 jon Exp $
 *
 * Function to spin some vectors under two generators
 * using intermediate files in a temporary directory.
 *
 */

#ifndef included__spf
#define included__spf

extern unsigned int spin(const char *in, const char *out, const char *a,
                         const char *b, const char *dir, const char *name);

#endif
