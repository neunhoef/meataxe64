/*
 * $Id: gen.c,v 1.1 2006/05/06 17:58:49 jon Exp $
 *
 * Functions to handle generator sets
 *
 */

#include "gen.h"
#include <stdio.h>

void cleanup_files(FILE **files, u32 count)
{
  if (NULL != files) {
    while (count > 0) {
      if (NULL != *files) {
        fclose(*files);
        *files = NULL;
      }
      files++;
      count--;
    }
  }
}

int unfinished(struct gen_struct *gens, unsigned int argc, u32 nor)
{
  while(argc > 0) {
    if (nor > gens[argc - 1].nor) {
      return 1;
    }
    argc--;
  }
  return 0;
}

int unfinishedf(genf gens, unsigned int argc, u32 nor)
{
  while(argc > 0) {
    if (nor > gens[argc - 1].nor) {
      return 1;
    }
    argc--;
  }
  return 0;
}
