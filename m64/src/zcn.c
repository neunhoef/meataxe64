/*
      zcn.c     meataxe-64 matrix concatenation
      =====     R. A. Parker    22.5.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "field.h"
#include "io.h"
#include "mfuns.h"

int main(int argc, const char *argv[])
{
  int nof = argc - 2;

  CLogCmd(argc, argv);
  if (argc < 3) {
    LogString(80,"usage zcn <m1> <m2> . . . <concat>");
    exit(14);
  }
  cat(argv + 1, argv[nof + 1], nof);
  return 0;
}

/******  end of zcn.c    ******/
