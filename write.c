/*
 * $Id: write.c,v 1.1 2001/08/28 21:39:44 jon Exp $
 *
 * Write a header
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "header.h"
#include "utils.h"
#include "write.h"

void write_text_header(FILE *fp, header h, const char *name)
{
  NOT_USED(fp);
  NOT_USED(h);
  NOT_USED(name);
  assert(NULL != h);
  assert(NULL != fp);
  assert(NULL != name);
}

void write_binary_header(FILE *fp, header h, const char *name)
{
  unsigned int nob;
  unsigned int prime;
  unsigned int nor;
  unsigned int noc;

  assert(NULL != h);
  assert(NULL != fp);
  assert(NULL != name);
  nob = header_get_nob(h);
  prime = header_get_prime(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);

  if (1 != fwrite(&nob, sizeof(unsigned int), 1, fp) ||
      1 != fwrite(&prime, sizeof(unsigned int), 1, fp) ||
      1 != fwrite(&nor, sizeof(unsigned int), 1, fp) ||
      1 != fwrite(&noc, sizeof(unsigned int), 1, fp)) {
    fprintf(stderr, "Failed to write header to binary output %s\n", name);
    exit(1);
  }
}
