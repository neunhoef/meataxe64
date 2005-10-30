/*
 * $Id: sizes.c,v 1.1 2005/10/30 10:53:47 jon Exp $
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
  printf("sizeof(char) = %" SIZE_F ", sizeof(short) = %" SIZE_F ", sizeof(int) = %" SIZE_F ", sizeof(long) = %" SIZE_F ", sizeof(size_t) = %" SIZE_F "\n",
         sizeof(char), sizeof(short), sizeof(int), sizeof(long), sizeof(size_t));
  return 0;
}
