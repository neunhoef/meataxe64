/*
 * $Id: rand.h,v 1.2 2004/08/21 13:22:31 jon Exp $
 *
 * Function to generate a random matrix
 *
 */

#ifndef included__rand
#define included__rand

extern int random(unsigned int prime, unsigned int nor, unsigned int noc,
                  const char *out, const char *name);

#define _ANSI_SOURCE /* Stop FreeBSD picking up rubbish */

#endif
