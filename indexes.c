/*
 * Compute indexes into rows, telling us how much can be ignored
 * because it's zero
 */

#include <errno.h>
#include "endian.h"
#include "indexes.h"
#include "utils.h"

/*
 * f is a file open just after the header. It will be returned thus
 * row is an allocated row into which we can read
 * indexes is an allocated array of length nor
 * nor is the number of rows in f
 * len is the length in words of a row
 */
int make_indexes(FILE *f, word *row, u32 *indexes, u32 nor, u32 len, const char *name, const char *m)
{
  u32 i;
  s64 pos = ftello64(f); /* Remember where we start */
  for (i = 0; i < nor; i++) {
    word *my_row = row, *end_row = row + len;
    errno = 0;
    if (0 == endian_read_row(f, my_row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: unable to read %s, terminating\n", name, m);
      return 0;
    }
    while (my_row < end_row) {
      if (0 != *my_row) {
        break;
      }
      my_row++;
    }
    indexes[i] = /*my_row - rows[0]*/0;
  }
  /* Now rewind f */
  if (0 != fseeko64(f, pos, SEEK_SET)) {
    fprintf(stderr, "%s: unable to rewind %s, terminating\n", name, m);
    return 0;
  }
  return 1;
}

