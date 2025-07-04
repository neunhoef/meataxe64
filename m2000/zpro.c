/*
 * $Id: zpro.c,v 1.7 2005/07/24 11:31:35 jon Exp $
 *
 * Print an orbit_set
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "endian.h"
#include "header.h"
#include "orbit.h"
#include "parse.h"
#include "read.h"
#include "utils.h"
#include "write.h"

static const char *name = "zpro";

static void pro_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s <in_file>\n", name, name, parse_usage());
}

int main(int argc, const char * const argv[])
{
  const char *in;
  FILE *inp;
  unsigned int nor;
  const header *h;
  orbit_set *orbits;
  endian_init();
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    pro_usage();
    exit(1);
  }
  in = argv[1];
  if (0 == open_and_read_binary_header(&inp, &h, in, name)) {
    exit(1);
  }
  nor = header_get_nor(h);
  if (0 == read_orbits(inp, nor, &orbits, in, name)) {
    fprintf(stderr, "%s: cannot read orbits, terminating\n", name);
    header_free(h);
    fclose(inp);
    exit(1);
  }
  fclose(inp);
  if (0 == write_text_header(stdout, h, name)) {
    fprintf(stderr, "%s: cannot write text header, terminating\n", name);
    header_free(h);
    fclose(inp);
    exit(1);
  }
  header_free(h);
  write_text_orbits(orbits);
  orbit_set_free(orbits);
  return 0;
}
