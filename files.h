/*
 * $Id: files.h,v 1.1 2001/09/30 21:49:18 jon Exp $
 *
 * file system stuff for unix
 *
 */

#ifndef included__files
#define included__files

extern int remove(const char *file);

extern int ren(const char *old, const char *new);

extern const char *pathname(const char *dirname, const char *filename);

#endif
