/*
      zspl.c     meataxe-64 matrix splice
      ======     J. G. Thackray    26.10.2024
*/

#include <stdio.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "mfuns.h"

int main(int argc, const char *argv[])
{
  unsigned int chops;

  CLogCmd(argc, argv);
  if (4 != argc) {
    LogString(80, "usage zspl tmp_base count output");
    exit(21);
  }
  chops = strtoul(argv[2], NULL, 0);
  if (chops >= 100) {
    fprintf(stderr, "%u chops is too many (>= 100), terminating\n", chops);
    exit(1);
  }
  splice(argv[1], chops, argv[3]);
  return 0;
}
