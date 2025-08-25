/*
      zrec.c     meataxe-64 recursive Echelize
      ======     J. G. Thackray 30.07.2025
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

static const char prog[] = "zrec";

int main(int argc, const char * const argv[])
{
  uint64_t rank;
  const char *tmp_root = tmp_name();

  argv = parse_line(argc, argv, &argc);
  CLogCmd(argc, argv);
  if (7 !=argc) {
    LogString(80,"usage zrec input row_select column_select multiplier cleaner remnant");
    exit(14);
  }
  rank = fRecurse_ECH(1, prog, tmp_root,
                      argv[1], argv[2], argv[3], argv[4],
                      argv[5], argv[6]);
  printf("%lu\n", rank);
  return 0;
}
