/*
 * $Id: zscript.c,v 1.3 2002/09/09 19:23:54 jon Exp $
 *
 * Compute a script in two generators
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "endian.h"
#include "memory.h"
#include "parse.h"
#include "script.h"

static const char *name = "zscript";

static void script_usage(void)
{
  fprintf(stderr, "%s: usage: %s <out_file> <tmp> <script> <memory> <a> <b>\n", name, name);
}

int main(int argc, const char * const argv[])
{
  const char *in1;
  const char *in2;
  const char *out;
  const char *tmp;
  const char *script;
  unsigned int memory = MEM_SIZE;

  argv = parse_line(argc, argv, &argc);
  if (7 != argc) {
    script_usage();
    exit(1);
  }
  in1 = argv[5];
  in2 = argv[6];
  out = argv[1];
  tmp = argv[2];
  script = argv[3];
  memory = strtoul(argv[4], NULL, 0);
  memory_init(name, memory);
  endian_init();
  if (0 == exec_script(in1, in2, out, tmp, script, name)) {
    exit(1);
  }
  memory_dispose();
  return 0;
}
