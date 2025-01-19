/*
 * $Id: endian.h,v 1.1 2017/10/02 20:00:57 jon Exp $
 *
 * Endian handling for meataxe
 *
 */

#ifndef included__endian
#define included__endian

#include <stdio.h>

extern u32 endian_invert_u32(u32);

extern int endian_write_word(word a, FILE *fp);

extern int endian_read_word(word *a, FILE *fp);

extern int endian_write_u32(u32 a, FILE *fp);

extern int endian_read_u32(u32 *a, FILE *fp);

extern int endian_write_u64(u64 a, FILE *fp);

extern int endian_read_u64(u64 *a, FILE *fp);

extern int endian_read_row(FILE *fp, word *row, u32 len);

extern int endian_write_row(FILE *fp, const word *row, u32 len);

extern void endian_skip_row(FILE *fp, u32 len);

extern int endian_read_matrix(FILE *fp, word **row,
                              u32 len, u32 nor);

extern int endian_write_matrix(FILE *fp, word **row,
                               u32 len, u32 nor);

extern int endian_copy_matrix(FILE *inp, FILE *outp, word *row,
                              u32 len, u32 nor);

extern void endian_init(void);

/* Extra functions for conversion between 32 and 64 bit systems */

extern int endian_read_u32_row(FILE *fp, u32 *row, u32 len);

extern int endian_read_u64_row(FILE *fp, u64 *row, u32 len);

extern int endian_write_u32_row(FILE *fp, const u32 *row, u32 len);

extern int endian_write_u64_row(FILE *fp, const u64 *row, u32 len);

#endif
