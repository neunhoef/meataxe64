/*
 * $Id: files.c,v 1.6 2019/01/21 08:32:35 jon Exp $
 *
 * file system stuff for unix
 *
 */

#include "../files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../utils.h"

int remove(const char *file)
{
  assert(NULL != file);
  return (unlink(file));
}

int ren(const char *old, const char *new)
{
  int retries = 100;
  while (retries-- >= 0) {
    struct stat buf1, buf2;
    int j;
    int k;
    (void)rename(old, new);
    /* Try to stat old file and new files */
    j = stat(old, &buf1);
    k = stat(new, &buf2);
    if (j) {
      /* Looks like old file not there */
      if (k) {
	/* New file not there either, just retry */
      } else {
	if (buf2.st_nlink == 1) {
	  /* We're happy */
	  return 0;
	} else {
	  /* Retry */
	}
      }
    } else {
      /* Old file still there, retry */
    }
  }
  fprintf(stderr, "rename of %s to %s failed, exiting\n", old, new);
  exit(1);
  return -1; /* Prevent compiler warning */
}

static int is_absolute(const char *filename)
{
  return (NULL != filename) && (('/' == *filename) || memcmp(filename+1, ":\\", 2) == 0);
}

const char *pathname(const char *dirname, const char *filename)
{
  char *result = my_malloc(strlen(dirname) + strlen(filename) + 2);
  if (is_absolute(filename)) {
    strcpy(result, filename);
  } else {
    strcpy(result, dirname);
    strcat(result, "/");
    strcat(result, filename);
  }
  return result;
}

unsigned long file_size(const char *filename)
{
  struct stat buf;
  int i = stat(filename, &buf);
  if (i) {
    return 0;
  } else {
    return buf.st_size;
  }
}
