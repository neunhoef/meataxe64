/*
 * $Id: extend.h,v 1.2 2005/06/22 21:52:53 jon Exp $
 *
 * Function to extend the field of a row
 *
 */

#ifndef included__extend
#define included__extend

extern int extend(const word *in, word *out,
                  u32 in_nob, u32 out_nob,
                  u32 in_prime, u32 out_prime,
                  u32 noc, const char *name);

#endif
