/*
 * $Id: sums_utils.h,v 1.2 2003/08/10 14:30:25 jon Exp $
 *
 * Utilities for sums, sumf etc
 *
 */

#ifndef included__sums_utils
#define included__sums_utils

/* Produce an element from its position number on the fly */
extern int make_element(unsigned int pos, unsigned int prime, unsigned int prime_power,
                        unsigned int i, const char **names, const char **elts, const char **elt_names , const char *name);

/* Return the number of the next generator, or -1 if none */
extern int next_gen(unsigned int cur_gen, unsigned int max_gen, char *gen, const unsigned int *orders, const char *word);

/* Return 1 if this word can be ignored due to being a multiple of a previously used word, otherwise 0 */
extern int ignore_word(unsigned int word, unsigned int max_prod, const char **words, unsigned int gen, unsigned int order, unsigned int prime);

typedef int (*accept)(unsigned int rank, unsigned int nor, const char *file, const char *form);

#endif
