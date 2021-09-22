/*
      zplain2.c     meataxe-64 convert two bitstrings and rem to plain format
      =========     J. G. Thackray   12.07.2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mfuns.h"
#include "io.h"

int main(int argc, const char *argv[])
{
  /* The root of the subspace */
  const char *sub_root, *zero_bs;
  unsigned int sub_root_len;
  char *sub_bs;
  char *sub_rem;
  header hdr;

  CLogCmd(argc, argv);
  /* Check command line <vecs> <output stem> */
  /* Must be exactly 4 args */
  if (argc != 4) {
    LogString(80,"usage zplain2 <zero bs> <subspace stem> <subspace plain form>");
    exit(21);
  }
  zero_bs = argv[1];
  sub_root = argv[2];
  sub_root_len = strlen(sub_root);
  sub_bs = malloc(sub_root_len + 4);
  sub_rem = malloc(sub_root_len + 5);
  strcpy(sub_bs, sub_root);
  strcat(sub_bs, ".bs");
  strcpy(sub_rem, sub_root);
  strcat(sub_rem, ".rem");
  EPeek(sub_rem, hdr.hdr);
  make_plain(zero_bs, sub_bs, sub_rem, argv[3], hdr.named.fdef);
  free(sub_bs);
  free(sub_rem);
  return 0;
}
