/* command.h */
/*
 * Interface to task manager (declaration)
 *
 */

#ifndef included__command
#define included__command

#include<stdio.h>

/* Command specifiers for potential remote operations */
typedef enum commands
{
  ADD,
  SUM,
  TRA,
  MUL,
  FLU
} command;

#define string_task(task) (((task) == ADD) ? "ad" : ((task) == MUL) ? "mu" : ((task) == TRA) ? "tr" : ((task) == SUM) ? "sum" : ((task) == FLU) ? "flu" : "unknown")

/* A unique id for tasks */
typedef unsigned long uid;

/* A unique id for temporaries */
typedef unsigned long t_uid;

/* Status types internal to the task list */
typedef enum task_status
{
  WAITING,	/* Not yet scheduled */
  IN_PROGRESS,	/* Being done by some slave */
  FINISHED,	/* Complete */
  FAILED,	/* Failed for mathematical reasons */
  FREE		/* In the command file but not yet assigned to a slave */
} task_status;

/* An id identifying the piece of work */
typedef unsigned long job;

typedef enum output_type
{
  TEMP,
  PERM
} output_type;

typedef struct output
{
  output_type type;
  union foobar
  {
    const char *name;
    t_uid uid;
  } value;
} output;

typedef enum input_type
{
  FILE_NAME,
  RESULT
} input_type;

typedef struct input
{
  input_type type;
  union foo
  {
    output output;
    struct bar
    {
      job job;
      unsigned int number;
    } result;
  } value;
} input;

typedef struct task
{
  job job;
  uid uid;
  task_status status;
  command command;
  unsigned int input_size;
  input *inputs;
  unsigned int output_size;
  output *outputs;
} task;

/* Initialise the task manager with a resource constraint and a temporary directory*/
extern void init_tasks(unsigned, const char *tmp_dir, const char *task_name);

/* Add a command to the task manager */
extern int add_command(job, command, unsigned, input *, unsigned, output *);

/* Generation of names and jobs */

extern job get_job(void);
extern t_uid get_tmp_id(void);

extern void copy_back(const char *new, const char *old, const char *task_name);

extern void prepend_task(const char *task_line, const char *task_name);

extern void append_task(const char *task_line, const char *task_name);

#endif
