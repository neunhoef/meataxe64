/*
 * $Id: utils.h,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Utilities for meataxe
 *
 */

#ifndef included__utils
#define included__utils

#define NOT_USED(_x) (void)(_x)

extern int is_a_prime_power(unsigned int);

extern int alloc_matrix(unsigned int nob, unsigned int noc,
                         unsigned int nor, unsigned int **res);

#endif
