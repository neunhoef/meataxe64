/*
 * $Id: write.c,v 1.2 2001/08/30 18:31:45 jon Exp $
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

void write_text_header(const FILE *fp, const header h)
{
  unsigned int prime, nod, noc, nor;
  char buf1[12];
  char buf2[12];
  char buf3[12];
  char buf4[12];

  assert(NULL != h);
  assert(NULL != fp);
  prime = header_get_prime(h);
  nod = header_get_nod(h);
  nor = header_get_nor(h);
  noc = header_get_noc(h);
  (void)sprintf(buf1, "%2d", nod);
  (void)sprintf(buf2, "%6d", prime);
  (void)sprintf(buf3, "%6d", nor);
  (void)sprintf(buf4, "%6d", noc);
  if (2 != fwrite(buf1, 1, 2, (FILE *)fp) ||
      6 != fwrite(buf2, 1, 6, (FILE *)fp) ||
      6 != fwrite(buf3, 1, 6, (FILE *)fp) ||
      6 != fwrite(buf4, 1, 6, (FILE *)fp) ||
      fputc('\n', (FILE *)fp) < 0) {
    fprintf(stderr, "Failed to write header to text output\n");
    exit(1);
  }
}

void write_binary_header(const FILE *fp, header h, const char *name)
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

  if (1 != fwrite(&nob, sizeof(unsigned int), 1, (FILE *)fp) ||
      1 != fwrite(&prime, sizeof(unsigned int), 1, (FILE *)fp) ||
      1 != fwrite(&nor, sizeof(unsigned int), 1, (FILE *)fp) ||
      1 != fwrite(&noc, sizeof(unsigned int), 1, (FILE *)fp)) {
    fprintf(stderr, "Failed to write header to binary output %s\n", name);
    exit(1);
  }
}
