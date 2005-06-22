/*
 * $Id: mtx.h,v 1.3 2005/06/22 21:52:53 jon Exp $
 *
 * Extended row operations for monster meataxe
 *
 */

#ifndef included__mtx
#define included__mtx

extern void put_row(u32 row_num, u32 total_cols,
                    u32 split_size, unsigned char *bits);

#endif
