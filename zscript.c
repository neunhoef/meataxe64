/*
 * $Id: zscript.c,v 1.1 2002/03/10 22:45:28 jon Exp $
 *
 * Compute a script in two generators
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "script.h"

static const char *name = "zscript";

static void script_usage(void)
{
  fprintf(stderr, "%s: usage: %s <a> <b> <out_file> <tmp> <script> [<memory>]\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;
  const char *tmp;
  const char *script;
  unsigned int memory = MEM_SIZE;

  if (6 != argc && 7 != argc) {
    script_usage();
    exit(1);
  }
  in1 = argv[1];
  in2 = argv[2];
  out = argv[3];
  tmp = argv[4];
  script = argv[5];
  if (7 == argc) {
    memory = strtoul(argv[6], NULL, 0);
  }
  memory_init(name, memory);
  endian_init();
  if (0 == exec_script(in1, in2, out, tmp, script, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
