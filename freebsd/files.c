/*
 * $Id: files.c,v 1.1 2004/08/21 13:22:32 jon Exp $
 *
 * file system stuff for freebsd, including large file handling
 *
 */

#include "../unix/files.c"
#include "utils.h"

FILE *fopen64(const char *name, const char *mode)
{
  return fopen(name, mode);
}

int fseeko64(FILE *f, long long off, int whence)
{
  return fseeko(f, off, whence);
}

long long ftello64(FILE *f)
{
  return ftello(f);
}

