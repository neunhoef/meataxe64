/*
 * $Id: write.h,v 1.6 2001/11/25 12:44:33 jon Exp $
 *
 * Writing a header for meataxe
 *
 */

#ifndef included__write
#define included__write

#include <stdio.h>
#include "header.h"

extern int write_text_header(FILE *, const header *);
extern int write_binary_header(FILE *, const header *, const char *);
extern int open_and_write_binary_header(FILE **, const header *, const char *, const char *);

#endif
