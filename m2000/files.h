/*
 * $Id: files.h,v 1.2 2002/01/06 16:35:48 jon Exp $
 *
 * file system stuff for unix
 *
 */

#ifndef included__files
#define included__files

extern int remove(const char *file);

extern int ren(const char *old, const char *new);

extern const char *pathname(const char *dirname, const char *filename);

extern unsigned long file_size(const char *filename);

#endif
