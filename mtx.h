/*
 * $Id: mtx.h,v 1.2 2001/09/30 17:51:20 jon Exp $
 *
 * Extended row operations for monster meataxe
 *
 */

#ifndef included__mtx
#define included__mtx

extern void put_row(unsigned int row_num, unsigned int total_cols,
                    unsigned int split_size, unsigned char *bits);

#endif
