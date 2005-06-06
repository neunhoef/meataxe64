/*
 * $Id: tcv.h,v 1.2 2005/06/06 08:01:37 jon Exp $
 *
 * Function to lift vectors from a tensor condensation representation
 *
 */

#ifndef included__tcv
#define included__tcv

extern int tco_lift(unsigned int s, const char *mults_l, const char *mults_r, const char *in,
                const char *out, int argc, const char *const *argv, const char *name);

#endif
