/*
 * $Id: read.h,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Reading a header for meataxe
 *
 */

#ifndef included__read
#define included__read

#include <stdio.h>

extern int read_text_header(FILE *, header *, const char *);
extern int read_binary_header(FILE *, header *, const char *);

#endif
