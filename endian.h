/*
 * $Id: endian.h,v 1.4 2001/09/12 23:13:04 jon Exp $
 *
 * Endian handling for meataxe
 *
 */

#ifndef included__endian
#define included__endian

extern int endian_write_int(unsigned int a, FILE *fp);

extern unsigned int endian_get_int(unsigned int i, const unsigned int *row);

extern int endian_read_row(FILE *fp, unsigned int *row, unsigned int len);

extern int endian_write_row(FILE *fp, const unsigned int *row, unsigned int len);

extern int endian_read_matrix(FILE *fp, unsigned int **row,
                              unsigned int len, unsigned int nor);

extern int endian_write_matrix(FILE *fp, unsigned int **row,
                               unsigned int len, unsigned int nor);

extern void endian_init(void);

#endif
