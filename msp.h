/*
 * $Id: msp.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Function to spin some vectors under multiple generators
 *
 */

#ifndef included__msp
#define included__msp

extern unsigned int spin(const char *in, const char *out,
                         unsigned int argc, const char * const args[],
                         const char *name);

#endif
