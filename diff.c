/*
 * $Id: diff.c,v 1.7 2005/06/22 21:52:53 jon Exp $
 *
 * Function to find the differences between two matrices
 *
 */

#include "diff.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "endian.h"
#include "header.h"
#include "maps.h"
#include "memory.h"
#include "read.h"
#include "utils.h"

int diff(const char *m1, const char *m2, const char *name)
{
  FILE *inp1 = NULL;
  FILE *inp2 = NULL;
  u32 prime, nob, noc, nor, len;
  u32 i;
  int res = 1;
  const header *h1, *h2;
  word *row1, *row2;

  assert(NULL != m1);
  assert(NULL != m2);
  assert(NULL != name);
  if (0 == open_and_read_binary_header(&inp1, &h1, m1, name) ||
      0 == open_and_read_binary_header(&inp2, &h2, m2, name)) {
    if (NULL != inp1) {
      fclose(inp1);
      header_free(h1);
    }
    return 0;
  }
  prime = header_get_prime(h1);
  nob = header_get_nob(h1);
  nor = header_get_nor(h1);
  noc = header_get_noc(h1);
  len = header_get_len(h1);
  if (1 == prime) {
    if (1 == header_get_prime(h2)) {
      /* Pair of maps */
      word *map1, *map2;
      if (header_get_prime(h2) != prime ||
        header_get_nob(h2) != nob ||
          header_get_noc(h2) != noc ||
          header_get_nor(h2) != nor) {
        fprintf(stderr, "%s header mismatch between %s and %s, terminating\n", name, m1, m2);
        fclose(inp1);
        fclose(inp2);
        header_free(h1);
        header_free(h2);
        return 0;
      }
      header_free(h1);
      header_free(h2);
      map1 = malloc_map(nor);
      map2 = malloc_map(nor);
      if (0 == read_map(inp1, nor, map1, name, m1) ||
          0 == read_map(inp2, nor, map2, name, m2)) {
        fclose(inp1);
        fclose(inp2);
        return 0;
      }
      fclose(inp1);
      fclose(inp2);
      if (0 != memcmp(map1, map2, nor * sizeof(word))) {
        return 0;
      } else {
        return 1;
      }
    } else {
      /* fall through to header mismatch case */
    }
  }
  if (header_get_prime(h2) != prime ||
      header_get_nob(h2) != nob ||
      header_get_noc(h2) != noc ||
      header_get_nor(h2) != nor) {
    fprintf(stderr, "%s header mismatch between %s and %s, terminating\n", name, m1, m2);
    fclose(inp1);
    fclose(inp2);
    header_free(h1);
    header_free(h2);
    return 0;
  }
  header_free(h1);
  header_free(h2);
  if (memory_rows(len, 1000) < 2) {
    fprintf(stderr, "%s cannot allocate rows for %s, %s, terminating\n", name, m1, m2);
    fclose(inp1);
    fclose(inp2);
    return 0;
  }
  row1 = memory_pointer_offset(0, 0, len);
  row2 = memory_pointer_offset(0, 1, len);
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(inp1, row1, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m1);
      fclose(inp1);
      fclose(inp2);
      return 0;
    }
    errno = 0;
    if (0 == endian_read_row(inp2, row2, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s cannot read row %d from %s, terminating\n", name, i, m2);
      fclose(inp1);
      fclose(inp2);
      return 0;
    }
    if (0 != memcmp(row1, row2, len * sizeof(word))) {
      u32 j, k = 0;
      for (j = 0; j < len; j++) {
        if (row1[j] != row2[j]) {
          printf("%s and %s differ in row %d near offset %d, with values 0x%x and 0x%x (diffs 0x%x)\n",
                 m1, m2, i, j * (bits_in_word / nob), (u32)row1[j], (u32)row2[j], (u32)(row1[j] ^ row2[j]));
          res = 0;
          k++;
          if (k > 10) {
            break;
          }
        }
      }
    }
  }
  fclose(inp1);
  fclose(inp2);
  return res;
}
