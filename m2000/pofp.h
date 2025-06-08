/*
 * $Id: pofp.h,v 1.1 2006/03/12 10:23:19 jon Exp $
 *
 * Function to compute fixed points of permutation in orbit
 *
 */

#ifndef included__pofp
#define included__pofp

extern u32 fixed_points_orbit(const char *orbit, unsigned int orbit_num, const char *out,
                              u32 argc, const char *const args[],
                              const char *name);

#endif
