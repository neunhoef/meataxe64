/*
 * $Id: endian.h,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Endian handling for meataxe
 *
 */

#ifndef included__endian
#define included__endian

extern int endian_write_int(unsigned int a, FILE *fp);

extern void endian_init(void);

#endif
