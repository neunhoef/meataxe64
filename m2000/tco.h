/*
 * $Id: tco.h,v 1.3 2005/06/22 21:52:54 jon Exp $
 *
 * Tensor condense one group element
 *
 */

#ifndef included__tco
#define included__tco

extern int tcondense(u32 s, const char *mults_l, const char *mults_r,
                     const char *irr, const char *end,
                     const char *left, const char *right, const char *out,
                     const char *in, u32 loop, u32 init,
                     int argc, const char *const *argv, const char *name);

#endif
