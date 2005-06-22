/*
 * $Id: sums_utils.h,v 1.5 2005/06/22 21:52:54 jon Exp $
 *
 * Utilities for sums, sumf etc
 *
 */

#ifndef included__sums_utils
#define included__sums_utils

/* Copy a string, discarding the old version if not NULL */
extern void copy_string(char **out, const char *in);

/* Generate an element script from its index */
extern char *make_elt_script(u32 prime_power, u32 cur_power,
                             u32 r, u32 l, u32 i,
                             u32 nod, const char **words);

/* Generate the element file name from its base and position */
char *make_elt_name(const char *base, u32 num);

/* Produce an element from its position number on the fly */
extern int make_element(u32 pos, u32 prime, u32 prime_power,
                        u32 i, const char **names, const char *base, const char *name);

/* Return the number of the next generator, or -1 if none */
extern int next_gen(u32 cur_gen, u32 max_gen, char *gen, const u32 *orders, const char *word);

/* Return 1 if this word can be ignored due to being a multiple of a previously used word, otherwise 0 */
extern int ignore_word(u32 word, u32 max_prod, const char **words, u32 gen, u32 order, u32 prime);

typedef int (*accept)(u32 rank, u32 nor, const char *file, const char *form);

#endif
