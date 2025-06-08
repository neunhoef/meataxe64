/*
 * $Id: nread.h,v 1.1 2012/06/17 10:41:29 jon Exp $
 *
 * Reading a header for meataxe 64
 *
 */

#ifndef included__nread
#define included__nread

#include <stdio.h>
#include "nheader.h"

extern int nread_binary_header(FILE *, const nheader **, const char *, const char *);
extern int open_and_read_binary_nheader(FILE **, const nheader **, const char *, const char *);

#endif /* included__nread */
