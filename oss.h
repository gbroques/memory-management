#ifndef OSS_H
#define OSS_H

#define MAX_PROCS 12

static void parse_command_options(int argc, char* argv[]);
static void print_help_message(char* executable_name);
static void free_shm_and_abort(int signum);
static void free_shm();
static int setup_interrupt();
static void fork_and_exec_children(pid_t* children);
static void fork_and_exec_child(pid_t* children, int index);
static void setup_clock_sem();
static void setup_mem_sems();
static void deallocate_mem_sems();
static void wait_for_all_children();

#endif