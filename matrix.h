/*
 * $Id: matrix.h,v 1.3 2001/09/18 23:15:46 jon Exp $
 *
 * Matrix manipulation for meataxe
 *
 */

#ifndef included__matrix
#define included__matrix

extern int matrix_malloc(unsigned int nor, void **);

extern void matrix_free(void *);

#endif
