/*
 * $Id: tra.h,v 1.2 2002/06/25 10:30:12 jon Exp $
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

#endif
