/*
 * $Id: mtx.h,v 1.1 2001/09/13 21:16:44 jon Exp $
 *
 * Extended row operations for monster meataxe
 *
 */

#ifndef included__mtx
#define included__mtx

extern void put_row(unsigned int row_num, unsigned int total_rows,
                    unsigned int split_size, unsigned char *bits);

#endif
