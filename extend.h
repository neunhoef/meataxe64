/*
 * $Id: extend.h,v 1.1 2002/01/22 08:40:24 jon Exp $
 *
 * Function to extend the field of a row
 *
 */

#ifndef included__extend
#define included__extend

extern int extend(const unsigned int *in, unsigned int *out,
                  unsigned int in_nob, unsigned int out_nob,
                  unsigned int in_prime, unsigned int out_prime,
                  unsigned int noc, const char *name);

#endif
