/*
 * $Id: endian.h,v 1.7 2001/11/29 01:13:09 jon Exp $
 *
 * Endian handling for meataxe
 *
 */

#ifndef included__endian
#define included__endian

#include <stdio.h>

extern unsigned int endian_invert(unsigned int);

extern int endian_write_int(unsigned int a, FILE *fp);

extern int endian_read_int(unsigned int *a, FILE *fp);

extern unsigned int endian_get_int(unsigned int i, const unsigned int *row);

extern int endian_read_row(FILE *fp, unsigned int *row, unsigned int len);

extern int endian_write_row(FILE *fp, const unsigned int *row, unsigned int len);

extern int endian_read_matrix(FILE *fp, unsigned int **row,
                              unsigned int len, unsigned int nor);

extern int endian_write_matrix(FILE *fp, unsigned int **row,
                               unsigned int len, unsigned int nor);

extern void endian_init(void);

#endif
