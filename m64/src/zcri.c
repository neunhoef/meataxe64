/*
      zrci.c     meataxe-64 collumn riffle identity
      ======     J. G. Thackray 01.09.2025
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "io.h"
#include "parse.h"
#include "funs.h"

int main(int argc, const char * const argv[])
{
  CLogCmd(argc, argv);
  argv = parse_line(argc, argv, &argc);
  if (4 !=argc) {
    LogString(80,"usage zcri column_select input output");
    exit(14);
  }
  fColumnRiffleIdentity(argv[1], 0, argv[2], 0, argv[3], 0);
  return 0;
}
