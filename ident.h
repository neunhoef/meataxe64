/*
 * $Id: ident.h,v 1.2 2002/01/14 23:43:45 jon Exp $
 *
 * Function to create identity matrix
 *
 */

#ifndef included__ident
#define included__ident

extern int ident(unsigned int prime, unsigned int nor, unsigned int noc, unsigned int elt,
                 const char *out, const char *name);

#endif
