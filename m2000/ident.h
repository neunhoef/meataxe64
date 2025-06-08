/*
 * $Id: ident.h,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Function to create identity matrix
 *
 */

#ifndef included__ident
#define included__ident

extern int ident(u32 prime, u32 nor, u32 noc, u32 elt,
                 const char *out, const char *name);

#endif
