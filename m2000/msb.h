/*
 * $Id: msb.h,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Function to spin some vectors under multiple generators to obtain a standard base
 *
 */

#ifndef included__msb
#define included__msb

extern u32 msb_spin(const char *in, const char *out,
                    unsigned int argc, const char * const args[],
                    const char *name);

#endif
