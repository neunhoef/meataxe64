/*
 * $Id: singular.h,v 1.2 2002/10/18 11:24:28 jon Exp $
 *
 * Function to find a singular vector, given a quadratic form
 *
 */

#ifndef included__singular
#define included__singular

#include <stdio.h>
#include "grease.h"

/* Return 0 if found, 1 on error and 255 on not found */
extern int singular_vector(unsigned int **rows, unsigned int **work,
                           unsigned int *out, unsigned int *out_num, FILE *formp,
                           unsigned int noc, unsigned int nor, unsigned int nob,
                           unsigned int len, unsigned int prime, grease grease,
                           const char *form, const char *name);

/* Return 0 if found, 1 on error and 255 on not found */
extern int singular(const char *space, const char *form, const char *out, const char *name);

#endif
