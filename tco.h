/*
 * $Id: tco.h,v 1.2 2004/07/31 07:50:15 jon Exp $
 *
 * Tensor condense one group element
 *
 */

#ifndef included__tco
#define included__tco

extern int tcondense(unsigned int s, const char *mults_l, const char *mults_r,
                     const char *irr, const char *end,
                     const char *left, const char *right, const char *out,
                     const char *in, unsigned int loop, unsigned int init,
                     int argc, const char *const *argv, const char *name);

#endif
