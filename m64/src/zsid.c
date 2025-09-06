/*
      zsid.c     meataxe-64 Generate a scaled identity matrix
      ======     J. G. Thackray   06.07.2021
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
  uint64_t fdef, nor, noc, elt;

  LogCmd(argc, argv);
  if (argc != 6) {
    LogString(80,"usage zsid m1 fdef nor noc elt");
    exit(14);
  }
  fdef = strtoull(argv[2], NULL, 0);
  nor = strtoull(argv[3], NULL, 0);
  noc = strtoull(argv[4], NULL, 0);
  elt = strtoull(argv[5], NULL, 0);
  if (elt >= fdef) {
    fprintf(stderr, "zsid: element %lu out of range for field %lu\n", elt, fdef);
    exit(1);
  }
  res = ident(fdef, nor, noc, elt, argv[1], 0);
  NOT_USED(res);
  return 0;
}
