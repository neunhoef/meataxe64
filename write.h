/*
 * $Id: write.h,v 1.3 2001/09/04 23:00:12 jon Exp $
 *
 * Writing a header for meataxe
 *
 */

#ifndef included__write
#define included__write

#include <stdio.h>

extern int write_text_header(const FILE *, const header);
extern int write_binary_header(const FILE *, const header, const char *);

#endif
