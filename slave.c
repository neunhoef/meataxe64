/*
 * $Id: slave.c,v 1.8 2002/01/14 23:43:45 jon Exp $
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
#include "endian.h"
#include "files.h"
#include "memory.h"
#include "mul.h"
#include "system.h"
#include "utils.h"

static const char *name = "meataxe slave";

static void slave_usage(void)
{
  fprintf(stderr, "%s: usage: %s <name> [<memory>]\n", name, name);
}

static int capable(const char *cmd, unsigned int length)
{
  return (strncmp(cmd, "kill", length) == 0 ||
          strncmp(cmd, "who", length) == 0 ||
          strncmp(cmd, "ad", length) == 0 ||
          strncmp(cmd, "mu", length) == 0);
}

static unsigned int parse_line(const char *(*line_ptrs)[], unsigned int (*lengths)[],
                               const char *line)
{
  unsigned int i = 0, j = 0, k;
  unsigned int len;
  assert(NULL != line);
  assert(NULL != line_ptrs);
  len = strlen(line);
  while (i < len) {
    i = skip_whitespace(i, line);
    assert(NULL != (*line_ptrs) + j);
    assert(NULL != (*lengths) + j);
    (*line_ptrs)[j] = line + i; /* Record start of word */
    k = skip_non_white(i, line);
    if (k != i) {
      (*lengths)[j] = k - i;
      j++;
      i = k;
      /* A non-empty word */
    } else {
      return j;
      /* End of line */
    }
  }
  assert(0); /* Shouldn't happen */
  return 0;
}

int main(int argc, const char *const argv[])
{
  unsigned int memory = MEM_SIZE;
  FILE *input, *output; 
  if (2 != argc && 3 != argc) {
    slave_usage();
    exit(1);
  }
  if (3 == argc) {
    memory = strtoul(argv[2], NULL, 0);
  }
  init_system();
  memory_init(name, memory);
  endian_init();
  while(1) {
    char line[MAX_LINE];
    const char *line_ptrs[MAX_LINE];
    unsigned int lengths[MAX_LINE];
    unsigned int words = 0;
    int free = 0;
    wait_lock(argv[1]);
    input = fopen(COMMAND_FILE, "rb");
    if (NULL == input) {
      /* Command file not created */
      release_lock();
      just_wait(10);
      continue;
    }
    output = fopen(COMMAND_COPY, "wb");
    if (NULL == output) {
      release_lock();
      fprintf(stderr, "Slave %s can't open %s\n, terminating\n", argv[1], COMMAND_COPY);
      exit(1);
    }
    while (get_task_line(line, input)) {
      /* Check for free command, else just copy back */
      words = parse_line(&line_ptrs, &lengths, line);
      if (1 <= words && 0 == strncmp(line_ptrs[0], "free", lengths[0])) {
        if (capable(line_ptrs[2], lengths[2])) {
          free = 1;
          break; /* Found a free job */
        }
      }
      /* Otherwise just copy the line over */
      fputs(line, output);
    }
    if (free) {
      unsigned long size1, size2;
      unsigned int a = strlen(argv[1]);
      unsigned int b = strlen("free");
      int is_kill = 1;
      NOT_USED(a);
      NOT_USED(b);
      assert(strlen("done") == b);
      if (0 != strncmp(line_ptrs[2], "kill", lengths[2])) {
        fprintf(output, "%s %s", argv[1], line_ptrs[1]);
        /* Only copy back if it's not kill */
        is_kill = 0;
      }
      copy_rest(output, input);
      fflush(output);
      fclose(output);
      fclose(input);
      size1 = file_size(COMMAND_FILE);
      size2 = file_size(COMMAND_COPY);
      if (is_kill) {
        assert(size1 == size2 + strlen(line_ptrs[0]));
      } else {
        assert(size1 + a == size2 + b);
      }
      input = fopen(COMMAND_COPY, "rb");
      if (NULL == input) {
        release_lock();
        fprintf(stderr, "Slave %s can't open %s\n, terminating\n", argv[1], COMMAND_COPY);
        exit(1);
      }
      output = fopen(COMMAND_FILE, "wb");
      if (NULL == output) {
        release_lock();
        fprintf(stderr, "Slave %s can't open %s\n, terminating\n", argv[1], COMMAND_FILE);
        exit(1);
      }
      copy_rest(output, input);
      fflush(output);
      fclose(output);
      fclose(input);
      size1 = file_size(COMMAND_FILE);
      size2 = file_size(COMMAND_COPY);
      assert(size1 == size2);
      /* Straight copy, why didn't we use rename? */
      release_lock();
      if (strncmp(line_ptrs[2], "kill", lengths[2]) == 0) {
        fprintf(stderr, "Slave %s killed by free kill\n", argv[1]);
        exit(0);
      } else if (strncmp(line_ptrs[2], "who", lengths[2]) == 0) {
        printf("Slave %s is here\n", argv[1]);
        just_wait(30);
      } else {
        char p[3][MAX_LINE];
        unsigned int i;
        for (i = 0; i < 3; i++) {
          if (i + 3 < words) {
            strncpy(p[i], line_ptrs[i + 3], lengths[i + 3]);
            p[i][lengths[i + 3]] = '\0';
          }
        }
        if (strncmp(line_ptrs[2], "ad", lengths[2]) == 0) {
          if (6 != words) {
            fprintf(stderr, "add parameters wrong\n");
            exit(1);
          }
          add(p[0], p[1], p[2], name);
        } else if (strncmp(line_ptrs[2], "mu", lengths[2]) == 0) {
          if (6 != words) {
            fprintf(stderr, "multiply parameters wrong\n");
            exit(1);
          }
          mul(p[0], p[1], p[2], name);
        } else {
          assert(0);
        }
      }
      wait_lock(argv[1]);
      input = fopen(COMMAND_FILE, "rb");
      if (NULL == input) {
        release_lock();
        fprintf(stderr, "Slave %s can't open %s\n, terminating\n", argv[1], COMMAND_FILE);
        exit(1);
      }
      output = fopen(COMMAND_COPY, "wb");
      if (NULL == output) {
        release_lock();
        fprintf(stderr, "Slave %s can't open %s\n, terminating\n", argv[1], COMMAND_COPY);
        exit(1);
      }
      while (get_task_line(line, input)) {
        words = parse_line(&line_ptrs, &lengths, line);
        if (0 != words && strncmp(argv[1], line_ptrs[0], lengths[0]) == 0) {
          fprintf(output, "done %s", line_ptrs[1]);
          break;
        } else {
          fputs(line, output);
        }
      }
      copy_rest(output, input);
      fflush(output);
      fclose(output);
      fclose(input);
      size1 = file_size(COMMAND_FILE);
      size2 = file_size(COMMAND_COPY);
      assert(size1 + b == size2 + a);
      /* Now copy to original command file */
      input = fopen(COMMAND_COPY, "rb");
      if (NULL == input) {
        release_lock();
        fprintf(stderr, "Slave %s can't open %s\n, terminating\n", argv[1], COMMAND_COPY);
        exit(1);
      }
      output = fopen(COMMAND_FILE, "wb");
      if (NULL == output) {
        release_lock();
        fprintf(stderr, "Slave %s can't open %s\n, terminating\n", argv[1], COMMAND_FILE);
        exit(1);
      }
      copy_rest(output, input);
      fflush(output);
      fclose(output);
      fclose(input);
    } else {
      /* No free job, don't worry about COMMAND_COPY */
      fclose(output);
      fclose(input);
    }
    release_lock();
    if (0 == free) {
      just_wait(10);
    }
  }
}
