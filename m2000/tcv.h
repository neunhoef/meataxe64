/*
 * $Id: tcv.h,v 1.3 2005/06/22 21:52:54 jon Exp $
 *
 * Function to lift vectors from a tensor condensation representation
 *
 */

#ifndef included__tcv
#define included__tcv

extern int tco_lift(u32 s, const char *mults_l, const char *mults_r, const char *in,
                    const char *out, int argc, const char *const *argv, const char *name);

#endif
