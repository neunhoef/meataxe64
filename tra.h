/*
 * $Id: tra.h,v 1.3 2003/06/21 21:56:57 jon Exp $
 *
 * Function to transpose a matrix
 *
 */

#ifndef included__tra
#define included__tra

extern int tra(const char *m1, const char *m2, const char *name);

/* In store transpose from rows1 to rows2 */
extern void tra_in_store(unsigned int **rows1, unsigned int **rows2,
                         unsigned int nor, unsigned int noc,
                         unsigned int nob, unsigned int col_len);

/* In situ transpose from rows to rows. Only available for nor = noc */
extern void tra_in_situ(unsigned int **rows, unsigned int nor, unsigned int nob);

#endif
