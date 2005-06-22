/*
 * $Id: rand.h,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Function to generate a random matrix
 *
 */

#ifndef included__rand
#define included__rand

extern int random(u32 prime, u32 nor, u32 noc,
                  const char *out, const char *name);

#define _ANSI_SOURCE /* Stop FreeBSD picking up rubbish */

#endif
