/*
 * $Id: slave.c,v 1.2 2001/09/26 22:04:47 jon Exp $
 *
 * Slave for extended operations
 * Based on zsl.c     MTX6 slave version 6.0.11 7.11.98 
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "add.h"
#include "command.h"
#include "memory.h"
#include "mul.h"
#include "system.h"

char buf[1000]; 
char buf1[1000];
const char *words[100], *words2[100];
char buf2[1000];

static const char *name = "meataxe slave";

static void slave_usage(void)
{
  fprintf(stderr, "%s: usage: %s <name>\n", name, name);
}

/* Read a line into buf */
/* Return 1 if we meet end of file before 1000 characters or '\n' */
/* Otherwise return 0 */
static int readline(FILE *f)
{
  const char *s = fgets(buf, 1000, f);
  if (s == NULL) return 1; else return 0;
}

/* Write a line of up to 1000 characters or a terminating '\n' */
static void writeline(FILE *f)
{
  const char *s = strchr(buf, '\n');
  unsigned int len, len1;
  assert (NULL != s);
  len = s + 1 - buf;
  len1 = fwrite(buf, 1, len, f); /* Write including the terminating '\n' */
  assert(len1  == len);
}

/* Some sort of line parsing */
static unsigned int parse(void)
{
  char b;
  unsigned int i, j;
  for(i = 0; i < 1000; i++)
    buf1[i] = buf[i];    
  buf1[999] = '\n'; 
  i = 0;
  j = 0;
  b = ' ';
  while('\n' != b) {
    if (' ' == b && ' ' != buf1[i] && '\n' != buf1[i])
      words[j++] = &buf1[i];
    if ((' ' == buf1[i] || '\n' == buf1[i]) && ' ' != b) 
      buf1[i] = '\0';
    if (j > 99) break;
    b = buf[i++];
  }
  return j;
}

static void unparse(unsigned int wordct, FILE *f)
{
  unsigned int i; 
  for(i = 0; i < wordct; i++) {
    fprintf(f, "%s ", words[i]);
  }
  fprintf(f, "\n");
}

static void grab(unsigned int wordct)
{
  unsigned int i, j;
  const char *pt;
  j = 0;
  for(i = 0; i < wordct; i++) {
    words2[i] = &buf2[j];
    pt = words[i];
    while('\n' != *pt && '\0' != *pt)
      buf2[j++] = *(pt++);
    buf2[j++] = '\0';
  }
}
int main(int argc, const char *const argv[])
{
  unsigned int i = 0, got = 1; 
  int cmdpar = 0;
  FILE *f1, *f2; 
  memory_init(name, 0);
  if (2 != argc) {
    slave_usage();
    exit(1);
  }
  init_system();
  while(1) {
/*  open files   */
    while (1) {
      if (got || check_signal()) {
        printf("Command file changed, checking\n");
        fflush(stdout);
        wait_lock(argv[1]);
        f1 = fopen("commands", "rb");
        if (NULL != f1) {
          break;
        } else {
          release_lock();
          just_wait(10);
        }
      } else {
        printf("Command file unchanged, waiting\n");
        fflush(stdout);
        just_wait(1);
      }
    }
    /* By now, we have acquired a lock and opened the commands file */
    f2 = fopen("commands1", "wb");
/*  copy till you find something interesting  */
    got = 0;
    while(1) {
      if (readline(f1)) {
        printf("Breaking for EOF\n");
        break; /* Break on EOF */
      }
      i = parse();
      if (0 == strcmp(words[0], argv[1])) {
        /* If we find our name against a task, mark it done */
        words[0] = "done";
        printf("Unparsing\n");
        unparse(i, f2);
        /* Write the line, then copy the rest over */
        printf("Copying the rest\n");
        copy_rest(f2, f1);
        break; /* Leave, with no reason to recheck or do any other action */
      }
      if (0 == strcmp(words[0], "free")) {
        printf("Slave %s - found a free command\n", argv[1]);
        /* Command awaiting some action, can we do it? */
        if (i > 2) {
          if (0 == strcmp(words[2], "kill")) {
            fclose(f1);
            fclose(f2);
            release_lock();
            fprintf(stderr, "Killed by free kill\n");
            exit(0);
          }
          if (0 == strcmp(words[2], "who") ||
              0 == strcmp(words[2], "mu") ||
              0 == strcmp(words[2], "ad")) {
            got = 1;
            break;
          }
        }
      }
      writeline(f2);
    }
/* Emerge with got = 1 <=> we found a line to process further */
/*  look at interesting line and process it  */
    if (1 == got) {
      printf("Slave %s - got something\n", argv[1]);
      grab(i);
      printf("grab %u\n", i);
      fflush(stdout);
      cmdpar = i-3;
      words[0] = argv[1];
      unparse(i, f2);
/*  and copy the rest of the file   */
      printf("copy rest of file\n");
      fflush(stdout);
      copy_rest(f2, f1);
    }
/*  in any case close and copy back   */
    fclose(f1);
    fflush(f2);
    fclose(f2);
    printf("copy back\n");
    fflush(stdout);
    f1 = fopen("commands1", "rb");
    f2 = fopen("commands", "wb");
    copy_rest(f2, f1);
    fclose(f1);
    fflush(f2);
    fclose(f2);
    printf("Done\n");
    fflush(stdout);
    release_lock();
    if (1 == got) {
      if (cmdpar < 0) {
        fprintf(stderr, "%s: not enough words on command line\n", name);
        exit(1);
      }
      if (0 == strcmp(words2[2], "who")) {
        printf("Slave %s is here\n", argv[1]);
        just_wait(30);
      }
      if (0 == strcmp(words2[2], "ad")) {
        if (cmdpar != 3) {
          fprintf(stderr, "add parameters wrong\n");
          exit(1);
        }
        add(words2[3], words2[4], words2[5], name);
      }
      if (0 == strcmp(words2[2], "mu")) {
        if (cmdpar != 3) {
          fprintf(stderr, "multiply parameters wrong\n");
          exit(1);
        }
        mul(words2[3], words2[4], words2[5], name);
      }
/* other things go here  */
    } else {
        just_wait(1);
    }
  }
}
