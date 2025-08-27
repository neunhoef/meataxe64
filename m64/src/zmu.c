// Copyright (C) Richard Parker   2017
// zmu.c     meataxe-64 Nikolaus version Multiply program

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "io.h"
#include "field.h"
#include "funs.h"
#include "util.h"
#include "parse.h"

int main(int argc, const char * const argv[])
{
  CLogCmd(argc, argv);
  argv = parse_line(argc, argv, &argc);
  if (argc != 4) {
    LogString(80,"usage zmu <m1> <m2> <product>");
    exit(14);
  }
  fMultiply(tmp_name(), argv[1], 0, argv[2], 0, argv[3], 0);    // just call fMultiply
  return 0;
}

/* end of zmu.c  */
