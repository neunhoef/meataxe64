/*
 * $Id: retract.h,v 1.1 2006/08/03 21:45:12 jon Exp $
 *
 * Function to retract the field of a row
 *
 */

#ifndef included__retract
#define included__retract

extern int retract(const word *in, word *out,
                   u32 in_nob, u32 out_nob,
                   u32 out_prime, u32 power,
                   u32 noc);

#endif
