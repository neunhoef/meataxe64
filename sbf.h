/*
 * $Id: sbf.h,v 1.1 2002/03/24 19:44:02 jon Exp $
 *
 * Function to spin some vectors under two generators to obtain a standard base
 * using intermediate files in a temporary directory.
 *
 */

#ifndef included__sb
#define included__sb

extern unsigned int spin(const char *in, const char *out, const char *a,
                         const char *b, const char *dir, const char *name);

#endif
