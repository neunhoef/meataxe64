/*
 * $Id: pco.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Permuation condense one group element
 *
 */

#ifndef included__pco
#define included__pco

extern int pcondense(const char *in1, const char *in2,
                     unsigned int field_order,
                     const char *out, const char *name);

#endif
