/*
 * $Id: orbit.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Functions for handling orbits
 *
 */

#ifndef included__orbit
#define included__orbit

#include <stdio.h>

typedef struct orbit
{
  unsigned int size;
  unsigned int *values;
  struct orbit *next;
} orbit;

typedef struct orbit_set
{
  unsigned int size;
  orbit *orbits;
} orbit_set;

/* Read binary form of an orbit_set */
extern int read_orbits(FILE *, unsigned int nor, orbit_set **,
                       const char *in, const char *name);

/* Write binary form of an orbit_set */
extern int write_orbits(FILE *, const orbit_set *,
                        const char *out, const char *name);

/* Write text form of an orbit */
extern void write_text_orbits(const orbit_set *);

/* Free an orbit set and all orbits therein */
extern void orbit_set_free(orbit_set *orbits);

/* Free an orbit */
extern void orbit_free(orbit *orb);

/* Free a chain of orbits */
extern void orbit_chain_free(orbit *orb);

#endif
