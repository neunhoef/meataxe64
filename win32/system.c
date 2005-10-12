/*
 * $Id: system.c,v 1.1 2005/10/12 18:20:31 jon Exp $
 *
 * system dependent stuff for locking etc
 */

#include "system.h"
#include "utils.h"
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <windows.h>

static char buf[256];

const char *tmp_name(void)
{
  int pid = _getpid();
  sprintf(buf, "tmp%d", pid);
  return buf;
}

void init_system(void)
{
}

void release_lock(void)
{
}

void wait_lock(const char *task_name)
{
  NOT_USED(task_name);
}

void just_wait(unsigned int i)
{
  NOT_USED(i);
}

int check_signal(void)
{
  return 0;
}
