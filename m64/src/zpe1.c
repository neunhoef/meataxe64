/*
      zpe1.c     meataxe-64 produce echelon form
      ======     J. G. Thackray 09.09.2025
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "parse.h"
#include "field.h"
#include "io.h"
#include "funs.h"
#include "util.h"
#include "utils.h"
#include "parse.h"

static const char prog[] = "zpe1";

int main(int argc, const char * const argv[])
{
  uint64_t rank;
  const char *tmp_root = tmp_name();
  char st[200];

  CLogCmd(argc, argv);
  argv = parse_line(argc, argv, &argc);
  if (4 !=argc) {
    LogString(80, "usage zpe1 input cs rem");
    exit(14);
  }
  rank = fRecurse_ECH(PE, 1, prog, tmp_root, argv[1], 0, "NULL", argv[2], "NULL",
                      "NULL", argv[3]);
  sprintf(st, "Rank %lu", rank);
  LogString(20, st);
  printf("%lu\n", rank);
  NOT_USED(rank);
  return 0;
}
