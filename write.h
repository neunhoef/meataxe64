/*
 * $Id: write.h,v 1.4 2001/09/12 23:13:04 jon Exp $
 *
 * Writing a header for meataxe
 *
 */

#ifndef included__write
#define included__write

#include <stdio.h>

extern int write_text_header(FILE *, const header *);
extern int write_binary_header(FILE *, const header *, const char *);

#endif
