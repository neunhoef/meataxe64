/*
 * $Id: tco.h,v 1.1 2002/08/27 17:12:38 jon Exp $
 *
 * Tensor condense one group element
 *
 */

#ifndef included__tco
#define included__tco

extern int tcondense(unsigned int s, const char *mults_l, const char *mults_r, const char *irr, const char *end,
                     const char *left, const char *right, const char *out,
                     int argc, const char *const *argv, const char *name);

#endif
