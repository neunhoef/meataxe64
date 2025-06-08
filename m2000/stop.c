/*
 * $Id: stop.c,v 1.1 2001/10/09 19:36:26 jon Exp $
 *
 * Stop for extended operations
 * Insert a kill command into the command file
 *
 */

#include <stdlib.h>
#include "command.h"
#include "utils.h"

static const char *name = "stop";

static void stop_usage(void)
{
  fprintf(stderr, "%s: usage: %s\n", name, name);
}

int main(int argc, const char *const argv[])
{
  NOT_USED(argv);
  if (1 != argc) {
    stop_usage();
    exit(1);
  }
  prepend_task("free 0 kill", name);
  return 0;
}
