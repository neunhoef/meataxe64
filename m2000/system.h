/* system.h */

#ifndef system_h
#define system_h

#define COMMAND_FILE "commands"

#define COMMAND_COPY "commands_copy"

#define COMMAND_DONE "commands_done"

#define LOCK "lock"

extern void just_wait(unsigned int i);

extern void wait_lock(const char *);

extern void release_lock(void);

extern int check_signal(void);

extern const char *tmp_name(void);

extern void init_system(void);

#endif
