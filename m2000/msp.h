/*
 * $Id: msp.h,v 1.2 2005/06/22 21:52:53 jon Exp $
 *
 * Function to spin some vectors under multiple generators
 *
 */

#ifndef included__msp
#define included__msp

extern u32 spin(const char *in, const char *out,
                u32 argc, const char *const args[],
                const char *name);

#endif
