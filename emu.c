/*
 * $Id: emu.c,v 1.8 2001/10/11 07:47:13 jon Exp $
 *
 * Exploded multiply
 *
 * Five arguments
 * 1) First addend dir
 * 2) Second addend dir
 * 3) Result dir
 * 4) Temporary results dir
 * 5) Maximum commands allowed
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "command.h"
#include "files.h"
#include "map.h"
#include "memory.h"
#include "utils.h"
#include "system.h"

static const char *name = "emu";

static void emu_usage(void)
{
  fprintf(stderr, "%s: usage: %s <dir1> <dir2> <dir3> <dir4> <limit>\n", name, name);
}

int main(int argc,  const char *const argv[])
{
  unsigned int col_pieces1, row_pieces1;
  unsigned int col_pieces2, row_pieces2;
  unsigned int i, j, k, limit;
  const char **names1, **names2, **names3;
  t_uid *tmp_ids;
  job *jobs;
  memory_init(name, 0);
  init_system();
  if (argc != 6) {
    emu_usage();
    exit(1);
  }
  limit = strtoul(argv[5], NULL, 0);
  if (0 == limit) {
    fprintf(stderr, "%s: cannot run with limit zero\n", name);
    exit(1);
  }
  init_tasks(limit, argv[4], "emu");
  /* Now look at the map file */
  input_map(name, argv[1], &col_pieces1, &row_pieces1, &names1);
  assert(NULL != names1);
  input_map(name, argv[2], &col_pieces2, &row_pieces2, &names2);
  assert(NULL != names2);
  if (col_pieces1 != row_pieces2) {
    fprintf(stderr, "%s: incompatible explosion points, terminating\n", name);
    exit(1);
  }
  if (0 == col_pieces1 || 0 == col_pieces2 || 0 == row_pieces1 || 0 == row_pieces2) {
    fprintf(stderr, "%s: cannot run with dimension zero, terminating\n", name);
    exit(1);
  }
  /* Now we have all relevant input names */
  /* We could now check for multiplication compatibility */
  /* Or we could assume the user has got it right, */
  /* and only fail when a spawned job fails */
  /* Now create the output names */
  output_map(name, argv[3], col_pieces2, row_pieces1, &names3);
  /* Now generate the temporary names for the multiplies */
  assert(NULL != names3);
  jobs = my_malloc(col_pieces1*col_pieces2*row_pieces1*sizeof(job));
  tmp_ids = my_malloc(col_pieces1*col_pieces2*row_pieces1*sizeof(t_uid));
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces2; j++) {
      for (k = 0; k < col_pieces1; k++) {
	tmp_ids[i*col_pieces1*col_pieces2 + j*col_pieces1 + k] = get_tmp_id();
	jobs[i*col_pieces1*col_pieces2 + j*col_pieces1 + k] = get_job();
      }
    }
  }
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces2; j++) {
      input *inputs = my_malloc(col_pieces1*sizeof(input));
      output *outputs = my_malloc(sizeof(output));
      /* Multiply and add to get a piece of the answer */
      if (col_pieces1 > 1) {
        for (k = 0; k < col_pieces1; k++) {
          input *inputs = my_malloc(2*sizeof(input));
          output *outputs = my_malloc(sizeof(output));
          inputs[0].type = FILE_NAME;
          inputs[0].value.output.type = PERM;
          inputs[0].value.output.value.name = pathname(argv[1], names1[i*col_pieces1 + k]);
          inputs[1].type = FILE_NAME;
          inputs[1].value.output.type = PERM;
          inputs[1].value.output.value.name = pathname(argv[2], names2[k*col_pieces2 + j]);
          outputs->type = TEMP;
          outputs->value.uid = tmp_ids[i*col_pieces1*col_pieces2 + j*col_pieces1 + k];
          add_command(jobs[i*col_pieces1*col_pieces2 + j*col_pieces1 + k],
                      MUL,
                      2,
                      inputs,
                      1,
                      outputs);
          /* Don't free inputs and outputs as add_command will hold onto them */
        }
        for (k = 0; k < col_pieces1; k++) {
          inputs[k].type = RESULT;
          inputs[k].value.result.job = jobs[i*col_pieces1*col_pieces2 + j*col_pieces1 + k];
          inputs[k].value.result.number = 0;
        }
        outputs->type = PERM;
        outputs->value.name = pathname(argv[3], names3[i*col_pieces2 + j]);
        add_command(get_job(), (col_pieces1 == 2) ? ADD : SUM, col_pieces1, inputs, 1, outputs);
      } else {
        /* 1x1 multiply */
        input *inputs = my_malloc(2*sizeof(input));
        output *outputs = my_malloc(sizeof(output));
        inputs[0].type = FILE_NAME;
        inputs[0].value.output.type = PERM;
        inputs[0].value.output.value.name = pathname(argv[1], names1[i]);
        inputs[1].type = FILE_NAME;
        inputs[1].value.output.type = PERM;
        inputs[1].value.output.value.name = pathname(argv[2], names2[j]);
        outputs->type = PERM;
        outputs->value.name = pathname(argv[3], names3[i*col_pieces2 + j]);
        add_command(jobs[i*col_pieces2 + j],
                    MUL,
                    2,
                    inputs,
                    1,
                    outputs);
        /* Don't free inputs and outputs as add_command will hold onto them */
      }
      /* Don't free inputs and outputs as add_command will hold onto them */
    }
  }
  /* Now force the answer */
  add_command(get_job(), FLU, 0, NULL, 0, NULL);
  return 0;
}
