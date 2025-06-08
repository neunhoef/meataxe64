/*
 * Function compute the sign of a permutation
 *
 */

#ifndef included__psign
#define included__psign

/* Return 0 ok, 1 on error. Print the sign */
extern int psign(const char *perm, const char *name);

/* Return the sign on a in memory permutation */
extern int psign_value(word *perm, u32 nor);

#endif
