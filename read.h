/*
 * $Id: read.h,v 1.2 2001/08/30 18:31:45 jon Exp $
 *
 * Reading a header for meataxe
 *
 */

#ifndef included__read
#define included__read

#include <stdio.h>

extern int read_text_header(const FILE *, header *, const char *);
extern int read_binary_header(const FILE *, header *, const char *);

#endif
