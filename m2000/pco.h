/*
 * $Id: pco.h,v 1.2 2005/06/22 21:52:53 jon Exp $
 *
 * Permuation condense one group element
 *
 */

#ifndef included__pco
#define included__pco

extern int pcondense(const char *in1, const char *in2,
                     u32 field_order,
                     const char *out, const char *name);

#endif
