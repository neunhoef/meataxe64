/*
 * $Id: mul.h,v 1.3 2001/11/21 01:06:29 jon Exp $
 *
 * Function to multiply two matrices to give a third
 *
 */

#ifndef included__mul
#define included__mul

#include <stdio.h>
#include "grease.h"

extern int mul(const char *m1, const char *m2, const char *m3, const char *name);

extern int mul_from_store(unsigned int **rows1, unsigned int **rows3,
                          FILE *inp, unsigned int noc, unsigned int len,
                          unsigned int nob, unsigned int nor, unsigned int prime,
                          grease grease, const char *m, const char *name);


#endif
