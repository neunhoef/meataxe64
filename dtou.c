/*
 * $Id: dtou.c,v 1.2 2001/09/04 23:00:12 jon Exp $
 *
 * Strip out DOS style line termination
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, const char * const argv[])
{
  FILE *inp, *outp;

  if (3 != argc) {
    fprintf(stderr, "dtou: bad usage(number of args)\n");
    exit(1);
  }
  inp = fopen(argv[1], "r");
  outp = fopen(argv[2], "w");
  if (NULL == inp || NULL == outp) {
    fprintf(stderr, "dtou: Can't open files %s, %s\n", argv[1], argv[2]);
    exit(1);
  }
  while (0 == feof(inp)) {
    int i = fgetc(inp);

    if (i >= 0) {
      if ('\r' != i)
        (void)fputc(i, outp);
    } else {
      fclose(inp);
      fclose(outp);
      break;
    }
  }
  return 0;
}
