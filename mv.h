/*
 * $Id: mv.h,v 1.2 2005/06/22 21:52:53 jon Exp $
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
extern void v_to_m(word *row_in, word **rows_out,
                   u32 nor1, u32 nor2,
                   u32 prime);

/* Convert vector to matrix
 * nor1 is number of rows, nor2 number of columns
 * expected use is efficient multiplication of vector by tensor
 * Hence the use of nor1 and nor2, rather than nor and noc
 * This routine cannot run in place
 */
extern void m_to_v(word **rows_in, word *row_out,
                   u32 nor, u32 noc,
                   u32 prime);

extern void create_pointers(word *row_in, word **rows_out,
                            u32 nor, u32 len,
                            u32 prime);

#endif
