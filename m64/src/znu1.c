/*
      znu1.c     meataxe-64 nullspace
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

static const char prog[] = "znu1";

int main(int argc, const char * const argv[])
{
  uint64_t rank, nullity;
  const char *tmp_root = tmp_name();
  uint64_t tmp_len = strlen(tmp_root);
  char st[200];
  const char *rs = mk_tmp(prog, tmp_root, tmp_len);
  const char *k = mk_tmp(prog, tmp_root, tmp_len);

  argv = parse_line(argc, argv, &argc);
  CLogCmd(argc, argv);
  argv = parse_line(argc, argv, &argc);
  if (3 !=argc) {
    LogString(80,"usage znu1 input nullspace");
    exit(14);
  }
  rank = fRecurse_ECH(NS, 1, prog, tmp_root, argv[1], 0, rs, "NULL", "NULL",
                      k, "NULL");
  nullity = fColumnRiffleIdentity(rs, 1, k, 1, argv[2], 0);
  sprintf(st, "Nullity %lu", nullity);
  LogString(20, st);
  printf("%lu\n", nullity);
  NOT_USED(rank);
  return 0;
}
