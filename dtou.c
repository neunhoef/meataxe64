/*
 * $Id: dtou.c,v 1.5 2002/06/28 08:39:16 jon Exp $
 *
 * Strip out DOS style line termination
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

static const char *name = "dtou";

static void dtou_usage(void)
{
  fprintf(stderr, "%s: usage: %s <in_file> <out_file>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  FILE *inp, *outp;

  if (3 != argc) {
    dtou_usage();
    exit(1);
  }
  errno = 0;
  inp = fopen(argv[1], "r");
  outp = fopen(argv[2], "w");
  if (NULL == inp || NULL == outp) {
    if ( 0 != errno) {
      perror(name);
    }
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
