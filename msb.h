/*
 * $Id: msb.h,v 1.2 2004/08/28 19:57:59 jon Exp $
 *
 * Function to spin some vectors under multiple generators to obtain a standard base
 *
 */

#ifndef included__msb
#define included__msb

extern unsigned int msb_spin(const char *in, const char *out,
                             unsigned int argc, const char * const args[],
                             const char *name);

#endif
