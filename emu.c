/* emu.c */
/*
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

#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "memory.h"
#include "utils.h"
#include "system.h"

static const char *name = "emu";

static void emu_usage(void)
{
  fprintf(stderr, "%s: usage: %s <dir1> <dir2> <dir3> <dir4> <limit>\n", name, name);
}

static unsigned int digits(unsigned long a)
{
  if (a < 10) {
    return 1;
  } else {
    return 1 + digits(a / 10);
  }
}

static int is_absolute(const char *filename)
{
  return (filename != NULL) && ((*filename == '/') || memcmp(filename+1, ":\\", 2) == 0);
}

static char *pathname(const char *dirname, const char *filename)
{
  char *result = my_malloc(strlen(dirname) + strlen(filename) + 2);
  if (is_absolute(filename)) {
    strcpy(result, filename);
  } else {
    strcpy(result, dirname);
    strcat(result, "/");
    strcat(result, filename);
  }
  return result;
}

static long hadcr = 0;

/******  subroutine to get an integer like FORTRAN does  */
static unsigned long getin(FILE *f, unsigned long a)
{
  int c;
  unsigned long i,j=0;
 
  if(hadcr == 1) return j;
  for(i=0;i<a;i++) {
    c = fgetc(f);
    if(c == '\n') {
      hadcr = 1;
      return j;
    }
    if(c < '0') c = '0';
    if(c > '9') c = '0';
    j = 10*j + (c-'0');
  }
  return j;
}
 
static void nextline(FILE *f)
{
  if(hadcr == 1) {
    hadcr=0;
    return;
  }
  while (fgetc(f) != '\n');
}
 
static char *get_str(FILE *f, char **name, unsigned int depth)
{
  int c = fgetc(f);
  if (my_isspace(c)) {
    *name = my_malloc(depth + 1);
    (*name)[depth] = '\0';
    return *name;
  } else {
    get_str(f, name, depth + 1);
    (*name)[depth] = c;
    return *name;
  }
}

int main(int argc,  char **argv)
{
  FILE *input1, *input2;
  FILE *output1;
  unsigned long col_pieces1, row_pieces1;
  unsigned long col_pieces2, row_pieces2;
  unsigned long i, j, k, limit;
  char **names1, **names2, **names3;
  t_uid *tmp_ids;
  char *temp;
  job *jobs;
  const char *m1, *m2, *m3;
  memory_init(name, 0);
  printf("emu starting\n");
  init_system();
  /******  First check the number of input arguments  */
  if (argc != 6) {
    emu_usage();
    exit(1);
  }
  /* Now get a look at the map file */
  limit = strtoul(argv[5], NULL, 0);
  init_tasks(limit, argv[4], "emu");
  m1 = pathname(argv[1], "map");
  input1 = fopen(m1, "rb");
  if (NULL == input1) {
    /*
    fprintf(stderr, "Attempting to open %s\n", m1);
    fflush(stderr);
    */
    fprintf(stderr, "%s: cannot open first input map %s", name, m1);
    exit(1);
  }
  m2 = pathname(argv[2], "map");
  input2 = fopen(m2, "rb");
  if (NULL == input2) {
    fprintf(stderr, "%s: cannot open second input map %s", name, m2);
    exit(1);
  }
  row_pieces1 = getin(input1, 6);
  col_pieces1 = getin(input1, 6);
  names1 = my_malloc(row_pieces1 * col_pieces1 * sizeof(char *));
  nextline(input1);
  row_pieces2 = getin(input2, 6);
  col_pieces2 = getin(input2, 6);
  nextline(input2);
  if (col_pieces1 != row_pieces2) {
    fprintf(stderr, "%s: incompatible explosion points, terminating\n", name);
    exit(1);
  }
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces1; j++) {
      names1[i * col_pieces1 + j] = get_str(input1, &temp, 0);
    }
    nextline(input1);
  }
  fclose(input1);
  names2 = my_malloc(row_pieces2 * col_pieces2 * sizeof(char *));
  for (i = 0; i < row_pieces2; i++) {
    for (j = 0; j < col_pieces2; j++) {
      names2[i * col_pieces2 + j] = get_str(input2, &temp, 0);
    }
    nextline(input2);
  }
  fclose(input2);
  /* Now we have all relevant input names */
  /* We could now check for multiplication compatibility */
  /* Or we could assume the user has got it right, */
  /* and only fail when a spawned job fails */
  /* Now create the output names */
  names3 = my_malloc(row_pieces1 * col_pieces2 * sizeof(char *));
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces2; j++) {
      char *name = my_malloc(digits(row_pieces1) + digits(col_pieces2) + 2); /* Make this more efficient ****/
      sprintf(name, "%lu_%lu", i, j);
      names3[i * col_pieces2 + j] = name;
    }
  }
  /* Now create the result map file */
  m3 = pathname(argv[3], "map");
  output1 = fopen(m3, "wb");
  if (output1 == NULL) {
    fprintf(stderr, "%s: cannot open output map %s", name, m3);
    exit(1);
  }
  fprintf(output1, "%6lu%6lu\n", row_pieces1, col_pieces2);
  for (i = 0; i < row_pieces1; i++) {
    for (j = 0; j < col_pieces2; j++) {
      fprintf(output1, "%s ", names3[i * col_pieces2 + j]);
    }
    fprintf(output1, "\n");
  }
  fclose(output1);
  /* Now generate the temporary names for the multiplies */
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
      }
      for (k = 0; k < col_pieces1; k++) {
	inputs[k].type = RESULT;
	inputs[k].value.result.job = jobs[i*col_pieces1*col_pieces2 + j*col_pieces1 + k];
	inputs[k].value.result.number = 0;
      }
      outputs->type = PERM;
      outputs->value.name = pathname(argv[3], names3[i*col_pieces2 + j]);
      add_command(get_job(), (col_pieces1 == 2) ? ADD : SUM, col_pieces1, inputs, 1, outputs);
    }
  }
  /* Now force the answer */
  add_command(get_job(), FLU, 0, NULL, 0, NULL);
  return 0;
}
