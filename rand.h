/*
 * $Id: rand.h,v 1.1 2002/07/20 12:55:21 jon Exp $
 *
 * Function to generate a random matrix
 *
 */

#ifndef included__rand
#define included__rand

extern int random(unsigned int prime, unsigned int nor, unsigned int noc,
                  const char *out, const char *name);

#endif
