/*
 * $Id: command.c,v 1.10 2001/12/15 20:47:27 jon Exp $
 *
 * Interface to task manager (definition)
 *
 */

#include "command.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "files.h"
#include "system.h"
#include "utils.h"

#define MAX_TASKS 1000000
static const char *task_name = NULL;
static task *tasks;
static task *outstanding[MAX_TASKS];
static task *consumers_waiting[MAX_TASKS];
static task *producers_waiting[MAX_TASKS];

static unsigned int task_size = 0;

static unsigned int max_commands = 0;
static const char *tmp_dir;
static unsigned long commands = 0;
static uid get_uid(void)
{
  static unsigned int uid_base=0;
  return (++uid_base);
}

job get_job(void)
{
  static unsigned int job_base=0;
  return (++job_base);
}

extern t_uid get_tmp_id(void)
{
  static unsigned int tmp_base=0;
  return (++tmp_base);
}

static task *assoc(task *tasks, unsigned int size, uid uid)
{
  unsigned int i=0;
  while (i < size) {
    if (tasks[i].uid == uid) return tasks+i;
    i++;
  }
  return NULL;
}

static int can_start(task *task)
{
  unsigned int i = 0;
  while(i < task->input_size) {
    assert(task->inputs[i].type == FILE_NAME || task->inputs[i].type == RESULT);
    if (task->inputs[i++].type != FILE_NAME) {
      return 0;
    }
  }
  return 1;
}

#define string_status(status) ((status) ==   WAITING) ? "waiting" : ((status) == IN_PROGRESS) ? "in progress" : ((status) == FINISHED) ? "finished" : ((status) == FAILED) ? "failed" : "free"

static void print_output_name(FILE *file, output *out)
{
  assert(TEMP == out->type || PERM == out->type);
  switch (out->type) {
  case TEMP:
    fprintf(file, " %s/tmp%lu", tmp_dir, out->value.uid);
    break;
  case PERM:
    fprintf(file, " %s", out->value.name);
    break;
  }
}

static void print_task(task *task)
{
  unsigned int i;
  input *inputs = task->inputs;
  output * outputs = task->outputs;
  printf("job %ld, uid %ld, status %s, function %s inputs", task->job, task->uid, string_status(task->status), (task->command == SUM) ? "sum" : string_task(task->command));
  for (i = 0; i < task->input_size; i++) {
    if (inputs[i].type == RESULT) {
      printf(" result %u of job %lu ", inputs[i].value.result.number, inputs[i].value.result.job);
    } else {
      assert(inputs[i].type == FILE_NAME);
      assert(TEMP == inputs[i].value.output.type || PERM == inputs[i].value.output.type);
      switch(inputs[i].value.output.type) {
      case TEMP:
	printf(" temporary");
      case PERM:
	break;
      }
      printf(" file");
      print_output_name(stdout, &inputs[i].value.output);
    }
  }
  printf(" outputs ");
  for (i = 0; i < task->output_size; i++) {
    assert(TEMP == outputs[i].type || PERM == outputs[i].type);
    switch(outputs[i].type) {
    case TEMP:
      printf("temporary ");
    case PERM:
      break;
    }
    printf("file");
    print_output_name(stdout, outputs + i);
  }
  printf("\n");
  fflush(stdout);
}

#define PAUSE 0

#if PAUSE
static void print_tasks(void)
{
  unsigned int i;
  for (i = 0; i < task_size; i++) {
    print_task(tasks + i);
  }
}

static void print_tasks_and_wait(void)
{
  print_tasks();
  just_wait(10);
}

static void print_tasks_and_pause(void)
{
  char temp[1000];
  print_tasks_and_wait();
  fgets(temp, 999, stdin);
}
#endif

static int sum_can_start(task *task)
{
  unsigned int i = 0;
  unsigned int j = 0;
  if (task->command == SUM) {
    while(i < task->input_size) {
      assert(task->inputs[i].type == FILE_NAME || task->inputs[i].type == RESULT);
      if (task->inputs[i++].type == FILE_NAME) {
	j++;
      }
    }
    return (j>=2);
  } else {
    return 0;
  }
}

static int produces(task *task)
{
  unsigned int i = 0;
  unsigned int j = 0;
  output *outputs = task->outputs;
  while (i < task->output_size) {
    assert(TEMP == outputs[i].type || PERM == outputs[i].type);
    if (outputs[i++].type == TEMP) {
      j++;
    }
  }
  return j;
}

static int consumes(task *task)
{
  switch (task->command) {
  case MUL:
    return 0;
  case ADD:
  case SUM:
    return task->input_size;
  default:
    fprintf(stderr, "Unknown command type in consumes\n");
    exit(1);
    return -1; /* Prevent compiler warning */
  }
}

static int producer(task *task)
{
  return produces(task) > consumes(task);
}

static int consumer(task *task)
{
  return !(producer(task));
}

/* Copy old back into new */
/* Assumes lock held */
void copy_back(const char *new, const char *old, const char *task_name)
{
  FILE *input, *output;
  assert(NULL != new);
  assert(NULL != old);
  assert(NULL != task_name);
  input = fopen(old, "rb");
  if (NULL == input) {
    release_lock();
    fprintf(stderr, "Slave %s can't open %s\n, terminating\n", task_name, old);
    exit(1);
  }
  output = fopen(new, "wb");
  if (NULL == output) {
    release_lock();
    fprintf(stderr, "Slave %s can't open %s\n, terminating\n", task_name, new);
    exit(1);
  }
  copy_rest(output, input);
  fflush(output);
  fclose(output);
  fclose(input);
}

static void add_task_sub(task *task, FILE *file)
{
  command command = task->command;
  unsigned int i=0;
  printf("Adding:- ");
  print_task(task);
  fprintf(file, "free %lu %s", task->uid, string_task(command));
  while (i < task->input_size) {
    if (task->inputs[i].type == RESULT) {
      release_lock();
      fprintf(stderr, "trying to add a task with undefined inputs\n");
      exit(1);
    } else {
      assert(task->inputs[i].type == FILE_NAME);
      print_output_name(file, &task->inputs[i++].value.output);
    }
  }
  i = 0;
  while (i < task->output_size) {
    print_output_name(file, task->outputs+i);
    i++;
  }
  fprintf(file, "\n");
  task->status = FREE;
}

static void add_tasks(int is_consumer, task *new_tasks[], unsigned int size)
{
  FILE *copy;
  FILE *new, *old;
  unsigned int i;
  wait_lock(task_name);
  copy = fopen(COMMAND_COPY, "wb");
  if (copy == NULL) {
    release_lock();
    fprintf(stderr, "Cannot create " COMMAND_COPY "\n");
    exit(1);
  }
  old = fopen(COMMAND_FILE, "rb");
  /* Copy the command file */
  if (old != NULL) {
    copy_rest(copy, old);
    fclose(old);
  }
  fflush(copy);
  fclose(copy);
  new = fopen(COMMAND_FILE, "wb");
  copy = fopen(COMMAND_COPY, "rb");
  if (new == NULL || copy == NULL) {
    release_lock();
    fprintf(stderr, "Cannot open " COMMAND_FILE " for write or " COMMAND_COPY " for read\n");
    exit(1);
  }
  if (is_consumer) {
    for (i = 0; i < size; i++) {
      add_task_sub(new_tasks[i], new);
    }
    copy_rest(new, copy);
  } else {
    copy_rest(new, copy);
    for (i = 0; i < size; i++) {
      add_task_sub(new_tasks[i], new);
    }
  }
  commands += size;
  fflush(new);
  fclose(new);
  fclose(copy);
  release_lock();
}

static void read_line(const char *line, uid *uid, task_status *status)
{
  char *temp;
  unsigned int i = skip_whitespace(0, line);
  unsigned int j = skip_non_white(i, line);
  unsigned int k = skip_whitespace(j, line);
  *status = (memcmp(line+i, "free", 4) == 0) ? FREE : (memcmp(line+i, "done", 4) == 0) ? FINISHED : (memcmp(line+i, "failed", 6) == 0) ? FAILED : IN_PROGRESS;
  *uid = strtoul(line+k, &temp, 0);
  if (temp == line+k) {
    fprintf(stderr, "Failed to read uid from command file\n");
    exit(1);
  }
}

static void process_line(const char *line, task *tasks, unsigned int size, FILE *copy, FILE *done, unsigned long *new_count, unsigned long *new_done)
{
  uid uid;
  task_status status;
  read_line(line, &uid, &status);
  switch(status) {
  case FREE:
  case WAITING:
  case IN_PROGRESS:
    {
      task *task = assoc(tasks, size, uid);
      if (task != NULL) {
	task->status = status;
      }
      (*new_count)++;
      fputs(line, copy);
    }
    break;
  case FINISHED:
    {
      task *task = assoc(tasks, size, uid);
      (*new_done)++;
      fputs(line, done);
      if (task != NULL) {
	if (task->status == status) {
	  /* No change of status */
	  printf("Curious, status for %ld unchanged\n", uid);
	  fflush(stdout);
	  fclose(copy);
	  fclose(done);
	  exit(1);
	  break;
	} else {
	  unsigned int i;
	  job job = task->job;
	  /* Update this task status */
	  task->status = status;	
	  /* Update all other tasks waiting on its results */
	  for (i = 0; i < size; i++) {
	    unsigned int j;
	    input *inputs = tasks[i].inputs;
	    for (j = 0; j < tasks[i].input_size; j++) {
	      switch (inputs[j].type) {
	      case RESULT:
		if (job == inputs[j].value.result.job) {
		  unsigned int number = inputs[j].value.result.number;
		  inputs[j].type = FILE_NAME;
		  if (inputs[j].value.result.number >= task->output_size) {
		    fprintf(stderr, "output number out of range\n");
                    exit(1);
		  }
		  if (number) {
		    printf("Strange, non-zero result number %u\n", number);
		    fflush(stdout);
		  }
		  printf("Updating job %lu input parameter %u with result", tasks[i].job, j);
		  print_output_name(stdout, task->outputs + number);
		  printf("%u from job %lu\n", number, job);
		  fflush(stdout);
		  inputs[j].type = FILE_NAME;
		  inputs[j].value.output.type = task->outputs[number].type;
		  inputs[j].value.output.value = task->outputs[number].value;
		}
              case FILE_NAME:
		break;
	      default:
                assert(0);
		break;
	      }
	    }
	  }
	  /* Delete all temporaries */
	  fflush(stdout);
	  for (i = 0; i < task->input_size; i++) {
	    char name[MAX_LINE];
            assert(TEMP == task->inputs[i].value.output.type ||
                   PERM == task->inputs[i].value.output.type);
	    switch(task->inputs[i].value.output.type) {
	    case TEMP:
	      printf("Deleting temporary");
	      print_output_name(stdout, &task->inputs[i].value.output);
	      printf("\n");
	      /***** Create the temporary name for remove */
	      sprintf(name, "%s/tmp%lu", tmp_dir, task->inputs[i].value.output.value.uid);
	      remove(name);
	      break;
	    default:
	      break;
	    }
	  }
	}
      } else {
	printf("uid %lu found in command file not one of my tasks\n", uid);
	fflush(stdout);
      }
    }
  break;
  case FAILED:
    fprintf(stderr, "Unexpected slave failure\n");
    exit(1);
  }
}

static void update(task *tasks, unsigned int size)
{
  unsigned long count = 0, new_count=0, new_done = 0;
  if (check_signal() == 0) {
    /* Command file unchanged */
    return;
  } else {
    FILE *input;
    /* Get the lock */
    wait_lock(task_name);
    input = fopen(COMMAND_FILE, "rb");
    if (input == NULL) {
      printf("Unexpected null result from fopen %s\n", COMMAND_FILE);
    } else {
      char line[MAX_LINE];
      FILE *copy = fopen(COMMAND_COPY, "wb");
      FILE *done = fopen(COMMAND_DONE, "ab");
      FILE *output;
      if (copy == NULL) {
	release_lock();
	fprintf(stderr, "Cannot create " COMMAND_COPY "\n");
        exit(1);
      }
      while(get_task_line(line, input)) {
	count ++;
	process_line(line, tasks, size, copy, done, &new_count, &new_done);
      };
      if (count != commands) {
	fprintf(stderr, "Incorrect number of commands found in command file (was %lu, should be %lu)\n", count, commands);
        release_lock();
	exit(1);
      }
      if (new_count + new_done != count) {
	printf("Incorrect number of commands copied and completed (was %lu + %lu, should be %lu)\n", new_count, new_done, commands);
	exit(1);
      }
      commands = new_count;
      fflush(copy);
      fclose(copy);
      fclose(done);
      fclose(input);
      /* Copy the command file back */
      input = fopen(COMMAND_COPY, "rb");
      output = fopen(COMMAND_FILE, "wb");
      if (input != NULL && output != NULL) {
	copy_rest(output, input);
	fflush(output);
	fclose(output);
	fclose(input);
      } else {
	release_lock();
	fprintf(stderr, "Cannot create either " COMMAND_COPY " or " COMMAND_FILE "\n");
        exit(1);
      }
    }
    /* Release the lock */
    release_lock();
    return;
  }
}

static void split(task *task1, task *tasks, unsigned int *size)
{
  t_uid tmp;
  task task2;
  input *inputs = task1->inputs;
  unsigned int i=0, j=0, k=0;
  if (*size >= MAX_TASKS) {
    fprintf(stderr, "Run out of tasks during split");
    exit(1);
  } else {
    while (i < task1->input_size) {
      assert(inputs[i].type == FILE_NAME || inputs[i].type == RESULT);
      if (inputs[i].type == FILE_NAME) {
	j = i;
	break;
      } else {
	i++;
      }
    };
    i++;
    while (i < task1->input_size) {
      assert(inputs[i].type == FILE_NAME || inputs[i].type == RESULT);
      if (inputs[i].type == FILE_NAME) {
	k = i;
	break;
      } else {
	i++;
      }
    };
    if (i == task1->input_size) {
      fprintf(stderr, "Split failed due to internal error\n");
      exit(1);
    }
  }
  tmp = get_tmp_id();
  task2.job = get_job();
  task2.uid = get_uid();
  task2.status = WAITING;
  task2.command = ADD;
  task2.inputs = my_malloc(sizeof(input) * 2);
  task2.input_size = 2;
  task2.output_size = 1;
  task2.outputs = my_malloc(sizeof(output));
  /* Fill in the inputs and outputs */
  task2.outputs[0].type = TEMP;
  task2.outputs[0].value.uid = tmp;
  task2.inputs[0].type = FILE_NAME;
  task2.inputs[0].value.output.type = task1->inputs[j].value.output.type;
  task2.inputs[0].value.output.value = task1->inputs[j].value.output.value;
  task2.inputs[1].type = FILE_NAME;
  task2.inputs[1].value.output.type = task1->inputs[k].value.output.type;
  task2.inputs[1].value.output.value = task1->inputs[k].value.output.value;
  /* Insert the new task */
  tasks[*size] = task2;
  /* Modify the old task */
  for (i = k+1; i < task1->input_size; i++) {
    task1->inputs[i-1] =  task1->inputs[i];
  }
  task1->inputs[j].type = RESULT;
  task1->inputs[j].value.result.job = task2.job;
  task1->inputs[j].value.result.number = 0;
  task1->input_size--;
  /* If short enough, replace by an add */
  if (task1->input_size == 2) {
    task1->command = ADD;
  }
  (*size)++;
}

#define free_arrays {free(outstanding); free(consumers_waiting); free(producers_waiting); }

static int schedule(task *tasks, unsigned int *size, unsigned int max_commands)
{
  unsigned int waiting_count = 0, consumer_count = 0, producer_count = 0, count = 0;
  unsigned int i;
  while (1) {
    int j = 0;
    for (i = 0; i < *size; i++) {
      task *task = tasks + i;
      if (sum_can_start(task)) {
	split(task, tasks, size);
	j = 1;
	break;
      }
    }
    if (!j) break;
  }
  for (i = 0; i < *size; i++) {
    task_status status = tasks[i].status;
    if (status == IN_PROGRESS || status == FREE) {
      count++;
    }
  }
  for (i = 0; i < *size; i++) {
    if (tasks[i].status == WAITING) {
      outstanding[waiting_count++] = tasks + i;
    }
  }
  for (i = 0; i < waiting_count; i++) {
    task *task = outstanding[i];
    if (consumer(task) && can_start(task)) {
      consumers_waiting[consumer_count++] = task;
    }
  }
  for (i = 0; i < waiting_count; i++) {
    task *task = outstanding[i];
    if (producer(task) && can_start(task)) {
      producers_waiting[producer_count++] = task;
    }
  }
  if (waiting_count == 0) {
    /* No tasks waiting to be added */
    /* See if any still in progress, if not we've finished */
    for (i = 0; i < *size; i++) {
      task_status status = tasks[i].status;
      if (status == FREE || status == IN_PROGRESS) {
        /* See if any have changed status */
        update(tasks, *size);
        return 1;
      }
    }
    /* We've finished all our tasks */
    return 0;
  } else {
    if (count < max_commands) {
      unsigned int available = max_commands - count;
      unsigned int consumers =
	(consumer_count <= available) ? consumer_count : available;
      /* Some tasks waiting to be added */
      /* Do all tasks additions */
      /* We need resource constraint here */
      add_tasks(1, consumers_waiting, consumers);
      available -= consumers;
      add_tasks(0, producers_waiting, (producer_count <= available) ? producer_count : available);
    } else {
    }
    /* Now check the time */
    update(tasks, *size);
#if PAUSE
    print_tasks_and_pause();
#endif
    return 1;
  }
}

int add_command(job job, command command, unsigned int input_size, input *inputs, unsigned int output_size, output *outputs)
{
  switch (command) {
  case FLU:
    /* Better do something now */
    {
      do {
	just_wait(1);
      } while(schedule(tasks, &task_size, max_commands));
    }
    wait_lock(task_name);
    ren(COMMAND_FILE, COMMAND_COPY);
    release_lock();
    break;
  default:
    /* Just add the job to the rest of them */
    if (task_size < MAX_TASKS) {
      task *task = tasks + task_size;
      task->command = command;
      task->job = job;
      task->uid = get_uid();
      task->input_size = input_size;
      task->output_size = output_size;
      task->inputs = inputs;
      task->outputs = outputs;
      task->status = WAITING;
      task_size++;
    } else {
      fprintf(stderr, "Too many m2000 commands\n");
      exit(1);
    }
    break;
  }
  return 0;
}

void prepend_task(const char *task_line, const char *task_name)
{
  FILE *input, *output;
  assert(NULL != task_line);
  wait_lock(task_name);
  do {
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
      fprintf(stderr, "Slave %s can't open %s\n, terminating\n", task_name, COMMAND_COPY);
      exit(1);
    }
    fprintf(output, "%s\n", task_line);
    copy_rest(output, input);
    fflush(output);
    fclose(output);
    fclose(input);
    /* Now copy to back original command file */
    copy_back(COMMAND_FILE, COMMAND_COPY, task_name);
    break;
  } while (1);
  release_lock();
}

void append_task(const char *task_line, const char *task_name)
{
  FILE *input, *output;
  assert(NULL != task_line);
  wait_lock(task_name);
  do {
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
      fprintf(stderr, "Slave %s can't open %s\n, terminating\n", task_name, COMMAND_COPY);
      exit(1);
    }
    copy_rest(output, input);
    fprintf(output, "%s\n", task_line);
    fflush(output);
    fclose(output);
    fclose(input);
    /* Now copy to back original command file */
    copy_back(COMMAND_FILE, COMMAND_COPY, task_name);
    break;
  } while (1);
  release_lock();
}


void init_tasks(unsigned int max, const char *dir, const char *name)
{
  max_commands = max;
  tmp_dir = dir;
  tasks = my_malloc(MAX_TASKS * sizeof(task));
  task_name = name;
}

