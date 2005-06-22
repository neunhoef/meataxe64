/*
 * $Id: orbit.h,v 1.2 2005/06/22 21:52:53 jon Exp $
 *
 * Functions for handling orbits
 *
 */

#ifndef included__orbit
#define included__orbit

#include <stdio.h>

typedef struct orbit
{
  u32 size;
  word *values;
  struct orbit *next;
} orbit;

typedef struct orbit_set
{
  u32 size;
  orbit *orbits;
} orbit_set;

/* Read binary form of an orbit_set */
extern int read_orbits(FILE *, u32 nor, orbit_set **,
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

/* Allocate an orbit set and all orbits therein, copying an existing orbit */
extern orbit_set *orbit_set_alloc(orbit_set *orbits);

#endif
