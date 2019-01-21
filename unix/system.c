/*
 * $Id: system.c,v 1.11 2019/01/21 08:32:35 jon Exp $
 *
 * system dependent stuff for locking etc
 */

#include "../system.h"
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include "../files.h"

void just_wait(unsigned int i)
{
  (void)sleep(i);
}

void wait_lock(const char *task_name)
{
  int retries = 100;
  char name[100];
  FILE *hitch;
  pid_t pid = getpid();
  struct hostent *h = gethostbyname("localhost");
  assert(NULL != task_name);
  assert(0 != h->h_length);
  sprintf(name, "meataxe_lock_%u_%s", (unsigned int)pid, h->h_name);
  for (;;) {
    retries--;
    if (retries <= 0) {
      fprintf(stderr, "lock acquisition failed, exiting\n");
      exit(1);
    }
    hitch = fopen(name, "wb");
    if (NULL == hitch) {
      printf("Cannot create lock file hitch, retrying\n");
      fflush(stdout);
    } else {
      fprintf(hitch, "Process %u, host %s, task %s\n", (unsigned int)pid, h->h_name, task_name);
      fclose(hitch);
      /* Now we have our unique name, try to link to the universal name */
      if (link(name, LOCK)) {
	/* Some sort of error */
	struct stat buf;
	int i = stat(name, &buf);
	printf("link failed, checking stat\n");
	fflush(stdout);
	if (i) {
	  /* Failed to stat hitch file */
	  printf("stat failed on hitch file, restarting lock\n");
	  fflush(stdout);
	  unlink(name);
	} else {
	  if (buf.st_nlink == 2) {
	    /* We succeeded but NFS told us the wrong answer */
	    break;
	  } else {
	    /* We failed to make the link, try again */
	    unlink(name);
	  }
	}
      } else {
	/* Success */
	/* Check NFS isn't lying */
	struct stat buf;
	int i = stat(name, &buf);
	if (i) {
	  /* Failed to stat hitch file */
	  printf("stat failed on hitch file, restarting lock\n");
	  fflush(stdout);
	  unlink(name);
	} else {
	  if (buf.st_nlink == 2) {
	    /* We succeeded but NFS told us the wrong answer */
	    break;
	  } else {
	    /* We failed to make the link, try again */
	    printf("unlink says success, but link says no\n");
	    fflush(stdout);
	    exit(1);
	  }
	}
      }
      sleep(1);
    }
  }
  unlink(name);
  /* If unlink fails here we don't really care */
}

void release_lock(void)
{
  if (remove(LOCK)) {
    printf("Release lock failed\n");
    fflush(stdout);
    switch(errno) {
    case EFAULT:
    case EACCES:
    case EPERM:
    case ENAMETOOLONG:
    case ENOENT:
    case ENOTDIR:
    case EISDIR:
    case ENOMEM:
    case EROFS:
      printf("Lock file fault %d (%s)\n", errno, strerror(errno));
    default:
      printf("Unexpected error %d (%s) deleting lock file\n", errno, strerror(errno));
    }
    fflush(stdout);
    exit(1);
  }
}

static time_t command_time = 0;

static int get_time(const char *name, time_t *time)
{
  struct stat buf;
  int i = stat(name, &buf);
  if (i) {
    return 0;
  } else {
    *time = buf.st_mtime;
    return 1;
  }
}

static unsigned int count = 0;

int check_signal(void)
{
  time_t tmp;
  int i = get_time(COMMAND_FILE, &tmp);
  int result = i;
  if (i != 0) {
    result = tmp != command_time;
    command_time = tmp;
  }
  if (result == 0) {
    count++;
    if (count == 10) {
      count = 0;
      result = 1;
    }
  }
  return result;
}

static char buf[256];

const char *tmp_name(void)
{
  pid_t pid = getpid();
  sprintf(buf, "tmp%u", (unsigned int)pid);
  return buf;
}

void init_system(void)
{
}
