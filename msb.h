/*
 * $Id: msb.h,v 1.1 2002/07/05 12:43:41 jon Exp $
 *
 * Function to spin some vectors under multiple generators to obtain a standard base
 *
 */

#ifndef included__msb
#define included__msb

extern unsigned int spin(const char *in, const char *out,
                         unsigned int argc, const char * const args[],
                         const char *name);

#endif
