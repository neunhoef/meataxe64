/*
 * Print the word size used by this meataxe
 * Current possible answers are 32 and 64
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "parse.h"
#include "utils.h"

static const char *name = "zword_size";

static void word_size_usage(void)
{
  fprintf(stderr, "%s: usage: %s %s\n", name, name, parse_usage());
}

int main(int argc, const char *const argv[])
{
  argv = parse_line(argc, argv, &argc);
  if (1 != argc) {
    word_size_usage();
    exit(1);
  }
  NOT_USED(argv);
  printf("%" SIZE_F "\n", (CHAR_BIT) * sizeof(word));
  return 0;
}
