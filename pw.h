/*
 * Function to search for peak words
 *
 */

#ifndef included__pw
#define included__pw

/*
 * n_irreds: how many irreducibles to search over
 * n_gens: how many generators per irreducible
 * peak: zero if a delta word suffices, otherwise we want a peak word
 * depth: how many group elements to make. The number of elements tested
 * will be q^depth, where q is the field order
 * orders: the orders of the individual elements of an irreducible
 * irred_names: the irreducibles name stems. Elements will be of the form
 * <name>_1, <name>_2 etc
 * nullities: the nullity required on each irreducible. Given that we probably
 * won't have a splitting field, these won't all be 1
 * words: a delta (or peak) word for each irreducible
 * Return 1 for success, 0 for fail
 */
extern int pw(u32 n_irreds, u32 n_gens, int peak, u32 depth, u32 *orders, char **irred_names, u32 *nullities, const char **words, const char *name);

#endif
