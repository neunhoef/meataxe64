/*
 * $Id: mv.h,v 1.1 2002/06/25 10:30:12 jon Exp $
 *
 * Function to convert rows to matrices and vv
 * Used for multiplication in tensor space
 *
 */

#ifndef included__mv
#define included__mv

/* Convert vector to matrix
 * nor1 is number of rows, nor2 number of columns
 * expected use is efficient multiplication of vector by tensor
 * Hence the use of nor1 and nor2, rather than nor and noc
 * This routine cannot run in place
 */
extern void v_to_m(unsigned int *row_in, unsigned int **rows_out,
                   unsigned int nor1, unsigned int nor2,
                   unsigned int prime);

/* Convert vector to matrix
 * nor1 is number of rows, nor2 number of columns
 * expected use is efficient multiplication of vector by tensor
 * Hence the use of nor1 and nor2, rather than nor and noc
 * This routine cannot run in place
 */
extern void m_to_v(unsigned int **rows_in, unsigned int *row_out,
                   unsigned int nor, unsigned int noc,
                   unsigned int prime);

extern void create_pointers(unsigned int *row_in, unsigned int **rows_out,
                            unsigned int nor, unsigned int len,
                            unsigned int prime);

#endif
