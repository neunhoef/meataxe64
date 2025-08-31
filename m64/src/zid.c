/*
      zid.c     meataxe-64 Generate an identity matrix
      =====     J. G. Thackray   30.10.2016 
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"
#include "slab.h"
#include "utils.h"
#include "mfuns.h"
 
int main(int argc,  char **argv)
{
  int res;
  uint64_t fdef, nor, noc;

  LogCmd(argc, argv);
  if (argc != 5) {
    LogString(80,"usage zid m1 fdef nor noc");
    exit(14);
  }
  fdef = strtoull(argv[2], NULL, 0);
  nor = strtoull(argv[3], NULL, 0);
  noc = strtoull(argv[4], NULL, 0);
  res = ident(fdef, nor, noc, 1, argv[1], 0);
  NOT_USED(res);
  return 0;
}
