/*
 * $Id: write.h,v 1.2 2001/08/30 18:31:45 jon Exp $
 *
 * Writing a header for meataxe
 *
 */

#ifndef included__write
#define included__write

#include <stdio.h>

extern void write_text_header(const FILE *, const header);
extern void write_binary_header(const FILE *, const header, const char *);

#endif
