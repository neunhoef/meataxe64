/*
 * $Id: zah.c,v 1.4 2002/06/28 08:39:16 jon Exp $
 *
 * Add a header to en intermediate file matrix
 * Essentially a disaster recovery program
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include "elements.h"
#include "endian.h"
#include "files.h"
#include "header.h"
#include "memory.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zah";

static void ah_usage(void)
{
  fprintf(stderr, "%s: usage: %s <text header> <in_file> <out_file> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in, *text_in;
  const char *out;
  FILE *inp, *text_inp;
  FILE *outp;
  unsigned int nor, len, *row, memory = 0;
  unsigned int i;
  long long ptr;
  const header *h;

  if (4 != argc && 5 != argc) {
    ah_usage();
    exit(1);
  }
  if (5 == argc) {
    memory = strtoul(argv[4], NULL, 0);
  }
  text_in = argv[1];
  in = argv[2];
  out = argv[3];
  memory_init(name, memory);
  endian_init();
  errno = 0;
  text_inp = fopen(text_in, "r");
  if (NULL == text_inp) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, text_in);
    exit(1);
  }
  errno = 0;
  inp = fopen64(in, "r");
  if (NULL == inp) {
    if ( 0 != errno) {
      perror(name);
    }
    fprintf(stderr, "%s: cannot open %s, terminating\n", name, in);
    fclose(text_inp);
    exit(1);
  }
  if (0 == read_text_header(text_inp, &h, text_in, name)) {
    fclose(text_inp);
    fclose(inp);
    exit(1);
  }
  fclose(text_inp);
  nor = header_get_nor(h);
  len = header_get_len(h);
  if (memory_rows(len, 1000) < 1) {
    fprintf(stderr, "%s: cannot obtain memory for one row of %s, terminating\n", name, in);
    fclose(inp);
    exit(1);
  }
  row = memory_pointer(0);
  if (0 == open_and_write_binary_header(&outp, h, out, name)) {
    fclose(inp);
    header_free(h);
    exit(1);
  }
  for (i = 0; i < nor; i++) {
    errno = 0;
    if (0 == endian_read_row(inp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot read row %d from %s, terminating\n", name, i, in);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
    errno = 0;
    if (0 == endian_write_row(outp, row, len)) {
      if ( 0 != errno) {
        perror(name);
      }
      fprintf(stderr, "%s: cannot write row %d to %s, terminating\n", name, i, out);
      fclose(inp);
      fclose(outp);
      exit(1);
    }
  }
  ptr = ftello64(inp);
  fseeko64(inp, 0, SEEK_END);
  if (ftello64(inp) != ptr) {
    fprintf(stderr, "%s: not all of input %s read, terminating\n", name, in);
    fclose(inp);
    fclose(outp);
    (void)remove(out);
    exit(1);
  }
  fclose(inp);
  fclose(outp);
  return 0;
}
