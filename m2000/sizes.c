/*
 * $Id: sizes.c,v 1.2 2005/10/30 11:25:54 jon Exp $
 *
 * Print the type sizes
 *
 */

#include <stdio.h>
#include "utils.h"

int main(int argc, const char * const argv[])
{
  NOT_USED(argc);
  NOT_USED(argv);
  printf("sizeof(char) = %" SIZE_F ", sizeof(short) = %" SIZE_F ", sizeof(int) = %" SIZE_F ", sizeof(long) = %" SIZE_F ", sizeof(size_t) = %" SIZE_F ", sizeof(u32) = %" SIZE_F ", sizeof(u64) = %" SIZE_F ", sizeof(word) = %" SIZE_F "\n",
         sizeof(char), sizeof(short), sizeof(int), sizeof(long), sizeof(size_t), sizeof(u32), sizeof(u64), sizeof(word));
  return 0;
}
