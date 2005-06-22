/*
 * $Id: singular.h,v 1.5 2005/06/22 21:52:54 jon Exp $
 *
 * Function to find a singular vector, given a quadratic form
 *
 */

#ifndef included__singular
#define included__singular

#include <stdio.h>
#include "rows.h"
#include "grease.h"

/* Return 0 if found, 1 on error and 255 on not found */
extern int singular_vector(row_ops *row_operations,
                           word **rows, word **work,
                           word *out, u32 *out_num, FILE *formp,
                           u32 noc, u32 nor, u32 nob,
                           u32 len, u32 prime, grease grease,
                           u32 index, const char *form, const char *name);

/* Return 0 if found, 1 on error and 255 on not found */
extern int singular(const char *space, const char *form, const char *out, const char *name);

#endif
