/*
 * $Id: sv.c,v 1.1 2006/03/04 09:02:06 jon Exp $
 *
 * Function to compute Schreier vector
 *
 */

#include "sv.h"
#include "gen.h"
#include "header.h"
#include "maps.h"
#include "read.h"
#include "utils.h"
#include "write.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void cleanup(u32 count, FILE **files)
{
  if (NULL != files) {
    while (count > 0) {
      if (NULL != *files) {
        fclose(*files);
      }
      files++;
      count--;
    }
  }
}

#define add_to_queue(pt) queue[queue_tail++] = pt
#define pop_from_queue queue[queue_head++]
#define is_empty_queue (queue_head == queue_tail)
#define assert_queue (queue_head <= queue_tail && queue_tail <= nor)

int sv(word point, const char *sv_out, const char *bv_out,
       u32 argc, const char *const args[],
       const char *name)
{
  FILE *bv_outp = NULL, *sv_outp = NULL, **files = NULL;
  u32 noc = 0, nor = 0, queue_head, queue_tail;
  word *queue, *schreier, *back;
  word **maps;
  unsigned int d;
  struct gen_struct *gens;
  header *h_out;
  assert(NULL != sv_out);
  assert(NULL != bv_out);
  assert(NULL != args);
  assert(0 < argc);
  assert(NULL != name);
  files = my_malloc(argc * sizeof(FILE *));
  gens = my_malloc(argc * sizeof(struct gen_struct));
  for (d = 1; d < argc; d++) {
    gens[d - 1].next = gens + d;
    files[d] = NULL;
  }
  gens[argc - 1].next = gens;
  files[0] = NULL;
  for (d = 0; d < argc; d++) {
    const char *gen_name = args[d];
    const header *h;
    if (0 == open_and_read_binary_header(files + d, &h, gen_name, name)) {
      cleanup(argc, files);
      return 1;
    }
    if (1 != header_get_prime(h)) {
      fprintf(stderr, "%s: non-permutation generator %s, terminating\n", name, gen_name);
      cleanup(argc, files);
      return 1;
    }
    gens[d].m = gen_name;
    gens[d].f = files[d];
    gens[d].nor = 0;
    if (0 == d) {
      noc = header_get_noc(h);
      nor = header_get_nor(h);
    }
    if (noc != header_get_noc(h) ||
        noc != header_get_nor(h)) {
      fprintf(stderr, "%s: incompatible parameters for %s, terminating\n",
              name, gen_name);
      cleanup(argc, files);
      return 1;
    }
    header_free(h);
  }
  if (point < 1 || point > nor) {
    fprintf(stderr, "%s: point %" W_F " is outside range 1 to %" U32_F "\n", name, point, nor);
    cleanup(argc, files);
    return 1;
  }
  point--;
  /* Read generators */
  maps = my_malloc(sizeof(*maps) * argc);
  for (d = 0; d < argc; d++) {
    maps[d] = malloc_map(nor);
  }
  for (d = 0; d < argc; d++) {
    int res = read_map(gens[d].f, nor, maps[d], name, gens[d].m);
    if (0 == res) {
      unsigned int j;
      for (j = 0; j < argc; j++) {
        map_free(maps[j]);
        free(maps);
      }
      return 1;
    }
  }
  cleanup(argc, files); /* Close all generators */
  queue = my_malloc(sizeof(*queue) * nor);
  schreier = my_malloc(sizeof(*schreier) * nor);
  back = my_malloc(sizeof(*back) * nor);
  for (d = 0; d < nor; d++) {
    schreier[d] = 0 - 1; /* Unset value */
    back[d] = 0 - 1;
  }
  queue_head = 0;
  queue_tail = 0;
  schreier[point] = 0 - 2;
  back[point] = 0 - 2;
  add_to_queue(point);
  while (!(is_empty_queue)) {
    point = pop_from_queue;
    assert(assert_queue);
    for (d = 0; d < argc; d++) {
      
      word pt = maps[d][point]; /* The image of point under g[d] */
      assert(pt < nor);
      if (0 == schreier[pt] + 1) {
        schreier[pt] = d;
        back[pt] = point;
        add_to_queue(pt);
      }
    }
  }
  for (d = 0; d < argc; d++) {
    map_free(maps[d]);
  }
  free(maps);
  free(queue);
  h_out = header_create(1, 0, 0, nor, nor);
  if (0 == open_and_write_binary_header(&sv_outp, h_out, sv_out, name)) {
    free(schreier);
    free(back);
    return 1;
  }
  if (0 == write_map(sv_outp, nor, schreier, name, sv_out)) {
    fclose(sv_outp);
    free(schreier);
    free(back);
    return 1;
  }
  fclose(sv_outp);
  free(schreier);
  if (0 == open_and_write_binary_header(&bv_outp, h_out, bv_out, name)) {
    free(back);
    return 1;
  }
  if (0 == write_map(bv_outp, nor, back, name, bv_out)) {
    fclose(bv_outp);
    free(back);
    return 1;
  }
  fclose(bv_outp);
  free(back);
  return 0;
}
