/*
      zrn.c     meataxe-64 Rank program
      =====     R. A. Parker 11.2.18
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "funs.h"
#include "util.h"
#include "parse.h"

int main(int argc, const char * const argv[])
{
  uint64_t rank;
  char st[200];
  const char *tmp_root = tmp_name();
  CLogCmd(argc, argv);
  argv = parse_line(argc, argv, &argc);
  if (2 != argc) {
    LogString(80,"usage zrn m");
    exit(14);
  }
  // make the temps properly!
  // also should probably be an fRank avoiding some work

  rank = fProduceNREF(tmp_root, argv[1], 0, "NULL", 1, "NULL", 1);
  sprintf(st,"Rank of %s is %lu", argv[1], rank);
  printf("%lu\n", rank);
  LogString(20, st);
  return 0;
}

/*  end of zrn.c    */
