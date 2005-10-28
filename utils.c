/*
 * $Id: utils.c,v 1.23 2005/10/28 22:58:08 jon Exp $
 *
 * Utils for meataxe
 *
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>

int assert_var_zero = 0;

u32 bits_in_word = CHAR_BIT * sizeof(word);

u32 bits_in_u32 = CHAR_BIT * sizeof(u32);

u32 bits_in_u64 = CHAR_BIT * sizeof(u64);

int my_isspace(int i)
{
  return (i == ' ') || (i == 9) || (i == 10) || (i == 13);
}

int my_isdigit(int i)
{
  return (i >= '0') && (i <= '9');
}

u32 read_decimal(const char *str, u32 len)
{
  char s[7];

  assert(0 < len);
  assert(len <= 6);
  assert(NULL != str);
  strncpy(s, str, len);
  s[len] = '\0';
  return strtoul(s, NULL, 0);
}

u32 getin(FILE *f, u32 a)
{
  u32 j = 0;
  char *s = my_malloc(a + 1);
  char *t = fgets(s, a + 1, f);
 
  if (NULL == t) {
    j = 0;
  } else {
    j = strtoul(s, NULL, 0);
  }
  free(s);
  return j;
}
 
static char *get_str_sub(FILE *f, u32 depth)
{
  int c = fgetc(f);
  char *out;
  if (c >= 0 && 0 == my_isspace(c)) {
    out = get_str_sub(f, depth + 1);
  } else {
    out = my_malloc(depth + 1);  
    c = '\0';
  }
  out[depth] = c;
  return out;
}

const char *get_str(FILE *f)
{
  int c = fgetc(f);
  while (my_isspace(c)) {
    c = fgetc(f);
  }
  if (c >= 0) {
    char *out = get_str_sub(f, 1);  
    *out = c;
    return out;
  } else {
    fprintf(stderr, "Failed to read string, terminating\n");
    exit(1);
  }
}

u32 bits_of(u32 n)
{
  u32 i = 0;
  assert(n >= 1);
  n -= 1;
  while(n > 0) {
    n >>= 1;
    i++;
  }
  return i;
}

/* Compute the number of decimal digits needed to represent 0 - n-1 */
u32 digits_of(u32 n)
{
  u32 i = 0;
  assert(n >= 1);
  n -= 1;
  while(n > 0) {
    n /= 10;
    i++;
  }
  return i;
}

void *my_malloc(size_t size)
{
  void *ret;
  assert(0 != size);
  errno = 0;
  ret = malloc(size);
  if (NULL == ret) {
    fprintf(stderr, "Failed to allocate %" SIZE_F " bytes with error reason '%s', terminating\n",
            size, (0 != errno) ? strerror(errno) : "unknown");
    exit(1);
  }
  return ret;
}

void copy_rest(FILE *new, FILE *old)
{
  char temp[1000];
  for (;;) {
    u32 i = fread(temp, 1, 1000, old);
    assert(i <= 1000);
    if (0 != i) {
      u32 j = fwrite(temp, 1, i, new);
      assert(j == i);
      NOT_USED(j);
    } else {
      break;
    }
  };
}

u32 skip_whitespace(u32 i, const char *chars)
{
  u32 j = strlen(chars);
  for (;;) {
    if (i >= j-1) {
      return i;
    } else {
      int k = chars[i];
      if (my_isspace(k)) {
	++i;
      } else {
	return i;
      }
    }
  }
}

u32 skip_non_white(u32 i, const char *chars)
{
  u32 j = strlen(chars);
  for (;;) {
    if (i >= j-1) {
      return i;
    } else {
      int k = chars[i];
      if (my_isspace(k)) {
	return i;
      } else {
	++i;
      }
    }
  }
}

int get_task_line(char *line, FILE *input)
{
  u32 i;
  char *poo = fgets(line, MAX_LINE-1, input);
  if (poo == NULL || strlen(line) <= 1) {
    return 0;
  } else {
    i = strlen(line);
    if (i >= MAX_LINE-2) {
      fprintf(stderr, "Command file line too long\n");
      return 0;
    }
    return 1;
  }
}

int int_pow(u32 n, u32 index, u32 *res)
{
  assert(0 != n);
  if (0 == index) {
    *res = 1;
    return 1;
  } else if (1 == index) {
    *res = n;
    return 1;
  } else {
    if (0 != int_pow(n, index-1, res)) {
      if (*res < (UINT_MAX / n)) {
        *res *= n;
        return 1;
      } else {
        return 0;
      }
    } else {
      return 0;
    }
  }
}

static unsigned char table[256];
static int table_initialised = 0;

static void init_table(void)
{
  u32 i;
  for (i = 0; i < 256; i++) {
    u32 j = i, k = 0, l;
    for (l = 0; l < 8; l++) {
      k = (k << 1) | (j & 1);
      j >>= 1;
    }
    table[i] = k;
  }
  table_initialised = 1;
}

unsigned char convert_char(unsigned char in)
{
  if (0 == table_initialised) {
    init_table();
  }
  return table[in];
}

int read_numbers(FILE *inp, u32 s, u32 *out)
{
  u32 i;
  assert(NULL != inp);
  assert(0 != s);
  assert(NULL != out);
  for (i = 0; i < s; i++) {
    fscanf(inp, "%u", out + i);
    if (ferror(inp) || feof(inp)) {
      return 0;
    }
  }
  return 1;
}
