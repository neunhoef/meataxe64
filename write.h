/*
 * $Id: write.h,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Writing a header for meataxe
 *
 */

#ifndef included__write
#define included__write

#include <stdio.h>

extern void write_text_header(FILE *, header, const char *);
extern void write_binary_header(FILE *, header, const char *);

#endif
