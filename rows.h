/*
 * $Id: rows.h,v 1.2 2001/09/02 22:16:41 jon Exp $
 *
 * Row manipulation for meataxe
 *
 */

#ifndef included__rows
#define included__rows

extern int row_add(const unsigned int *, const unsigned int *,
                   unsigned int *, unsigned int,
                   unsigned int, unsigned int);

extern void row_copy(const unsigned int *, unsigned int *,
                     unsigned int, unsigned int);

extern int row_malloc(unsigned int nob, unsigned int noc, unsigned int **row);

extern void row_init(unsigned int *, unsigned int nob, unsigned int noc);

#endif
