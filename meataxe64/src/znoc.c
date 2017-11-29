/*
      znoc.c     meataxe-64 Utility routines
      ======     J. G. Thackray   28.10.2016
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "field.h"
#include "io.h"
 
int main(int argc,  char **argv)
{
  EFIL *e;
  uint64 hdr[5];
  LogCmd(argc,argv);
  if (argc != 2) {
    LogString(80,"usage znoc <file>");
    exit(21);
  }
  e = ERHdr(argv[1], hdr);
  (void)e;
  printf("%lu\n", hdr[3]);
  exit(0);
}
