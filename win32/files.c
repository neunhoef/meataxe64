/*
 * $Id: files.c,v 1.2 2019/01/21 08:32:35 jon Exp $
 *
 * file system stuff for win32
 *
 */

#include "files.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <windows.h>

int ren(const char *old, const char *new)
{
  return rename(old, new);
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
  HANDLE hFile = CreateFile(filename,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    return 0;
  } else {
    DWORD high;
    DWORD size = GetFileSize(&high, hFile);
    (void)CloseHandle(hFile);
    return size;
  }
}
