/* system.c */
/*
 * system dependent stuff for locking etc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include "system.h"

void just_wait(unsigned int i)
{
  (void)sleep(i);
}

void wait_lock(const char *task_name)
{
  int retries = 100;
  char name[100];
  FILE *foo;
  __pid_t pid = getpid();
  struct hostent *h = gethostbyname("localhost");
#if 0
  unsigned long hostid;

 = (unsigned long)gethostid();
#endif
  assert(0 != h->h_length);
  sprintf(name, "meataxe_lock_%u_%s", pid, h->h_addr_list[0]);
  while(1) {
    retries--;
    if (retries <= 0) {
      printf("lock acquisition failed, exiting\n");
      fflush(stdout);
      exit(1);
    }
#if 0
    printf("Acquiring lock\n");
#endif
    fflush(stdout);
    foo = fopen(name, "wb");
    if (foo == NULL) {
      printf("Cannot create lock file hitch, retrying\n");
      fflush(stdout);
    } else {
      fprintf(foo, "Process %u, host %s, task %s\n", pid, h->h_addr_list[0], task_name);
      fclose(foo);
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
#if 0
	printf("link succeeded, checking stat\n");
#endif
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
#if 0
  printf("Acquired lock\n");
#endif
  fflush(stdout);
}

int remove(const char *file)
{
  return (unlink(file));
}

int ren(const char *old, const char *new)
{
  int retries = 100;
  while (retries-- >= 0) {
    struct stat buf1, buf2;
    int j;
    int k;
    (void)rename(old, new);
    /* Try to stat old file and new files */
    j = stat(old, &buf1);
    k = stat(new, &buf2);
    if (j) {
      /* Looks like old file not there */
      if (k) {
	/* New file not there either, just retry */
      } else {
	if (buf2.st_nlink == 1) {
	  /* We're happy */
	  return 0;
	} else {
	  /* Retry */
	}
      }
    } else {
      /* Old file still there, retry */
    }
  }
  printf("rename of %s to %s failed, exiting\n", old, new);
  exit(1);
  return -1; /* Prevent compiler warning */
}

void release_lock(void)
{
#if 0
  printf("Releasing lock\n");
#endif
  fflush(stdout);
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
      fflush(stdout);
      exit(1);
    default:
      printf("Unexpected error %d (%s) deleting lock file\n", errno, strerror(errno));
      fflush(stdout);
      exit(1);
    }
  } else {
#if 0
    printf("Released lock\n");
#endif
    fflush(stdout);
  }
}

static __time_t command_time = 0;

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

void signal(void)
{
}

void init_system(void)
{
}
