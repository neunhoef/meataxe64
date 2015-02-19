/*
 * $Id: singular.h,v 1.7 2015/02/19 09:06:09 jon Exp $
 *
 * Function to find a singular vector, given a quadratic form
 *
 */

#ifndef included__singular
#define included__singular

#include <stdio.h>
#include "rows.h"
#include "grease.h"

/*
 * row_ops: the usual operations on vectors
 * rows: A pointer to the start of the search space
 * work: Some workspace we can use
 * out: the singular vector we've found, if we find one
 * out_num: the number of a row involved with non zero coefficient in the output
 * formp: the quadratic form we're using
 * noc: number of columns in a vector
 * nor: the size of the search space we're given
 * nob: usual meaning
 * len: usual meaning
 * prime: the field order
 * grease: the grease tables to be used
 * index: A pointer to where the first non-zero element might be
 *        used to optimise multiplication
 * form: the name of the file containing the quadratic form
 * name: the name of the outer calling program, eg zsign
 * Return 0 if found, 1 on error and 255 on not found
 */
extern int singular_vector(row_ops *row_operations,
                           word **rows, word **work,
                           word *out, u32 *out_num, FILE *formp,
                           u32 noc, u32 nor, u32 nob,
                           u32 len, u32 prime, grease grease,
                           u32 index, u32 *indexes, const char *form, const char *name);

/* Return 0 if found, 1 on error and 255 on not found */
extern int singular(const char *space, const char *form, const char *out, const char *name);

#endif
