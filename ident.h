/*
 * $Id: ident.h,v 1.1 2001/10/10 23:19:42 jon Exp $
 *
 * Function to create identity matrix
 *
 */

#ifndef included__ident
#define included__ident

extern int ident(unsigned int prime, unsigned int nor, unsigned int noc,
                 const char *out, const char *name);

#endif
