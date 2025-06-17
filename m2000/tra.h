/*
 * $Id: tra.h,v 1.4 2005/06/22 21:52:54 jon Exp $
 *
 * Function to transpose a matrix
 *
 */

#ifndef included__tra
#define included__tra

extern int tra(const char *m1, const char *m2, const char *name);

/* In store transpose from rows1 to rows2 */
extern void tra_in_store(word **rows1, word **rows2,
                         u32 nor, u32 noc,
                         u32 nob, u32 col_len);

/* In situ transpose from rows to rows. Only available for nor = noc */
extern void tra_in_situ(word **rows, u32 nor, u32 nob);

#endif
