/*
 * $Id: sums_utils.h,v 1.1 2003/08/04 20:41:57 jon Exp $
 *
 * Utilities for sums, sumf etc
 *
 */

#ifndef included__sums_utils
#define included__sums_utils

/* Return the number of the next generator, or -1 if none */
extern int next_gen(unsigned int cur_gen, unsigned int max_gen, char *gen, const unsigned int *orders, const char *word);

/* Return 1 if this word can be ignored due to being a multiple of a previously used word, otherwise 0 */
extern int ignore_word(unsigned int word, unsigned int max_prod, const char **words, unsigned int gen, unsigned int order, unsigned int prime);

typedef int (*accept)(unsigned int rank, unsigned int nor, const char *file, const char *form);

#endif
