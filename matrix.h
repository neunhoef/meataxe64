/*
 * $Id: matrix.h,v 1.1 2001/09/02 22:16:41 jon Exp $
 *
 * Matrix manipulation for meataxe
 *
 */

#ifndef included__matrix
#define included__matrix

extern int matrix_malloc(unsigned int nob, unsigned int nor,
                         unsigned int noc, unsigned int ***row);

#endif
