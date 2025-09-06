/*
      zrn1.c     meataxe-64 rank using recursive echelise
      ======     J. G. Thackray 01.09.2025
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

static const char prog[] = "zrn1";

int main(int argc, const char * const argv[])
{
  uint64_t rank;
  char st[200];
  const char *tmp_root = tmp_name();

  CLogCmd(argc, argv);
  argv = parse_line(argc, argv, &argc);
  if (2 !=argc) {
    LogString(80,"usage zrn1 input");
    exit(14);
  }
  rank = fRecurse_ECH(RN, 1, prog, tmp_root, argv[1], 0, "NULL", "NULL", "NULL",
                      "NULL", "NULL");
  sprintf(st,"Rank of %s is %lu", argv[1], rank);
  LogString(20, st);
  printf("%lu\n", rank);
  return 0;
}
