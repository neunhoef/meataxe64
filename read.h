/*
 * $Id: read.h,v 1.3 2001/09/12 23:13:04 jon Exp $
 *
 * Reading a header for meataxe
 *
 */

#ifndef included__read
#define included__read

#include <stdio.h>

extern int read_text_header(FILE *, const header **, const char *);
extern int read_binary_header(FILE *, const header **, const char *);

#endif
