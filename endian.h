/*
 * $Id: endian.h,v 1.3 2001/09/02 22:16:41 jon Exp $
 *
 * Endian handling for meataxe
 *
 */

#ifndef included__endian
#define included__endian

extern int endian_write_int(unsigned int a, const FILE *fp);

extern unsigned int endian_get_int(unsigned int i, const unsigned int *row);

extern int endian_read_row(const FILE *fp, unsigned int *row,
                           unsigned int nob, unsigned int noc);

extern int endian_write_row(const FILE *fp, const unsigned int *row,
                            unsigned int nob, unsigned int noc);

extern int endian_read_matrix(const FILE *fp, unsigned int **row,
                              unsigned int nob, unsigned int noc,
                              unsigned int nor);

extern int endian_write_matrix(const FILE *fp, unsigned int **row,
                               unsigned int nob, unsigned int noc,
                               unsigned int nor);

extern void endian_init(void);

#endif
