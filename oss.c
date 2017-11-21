/**
 * Operating System Simulator
 *
 * Copyright (c) 2017 G Brenden Roques
 */

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "oss.h"
#include "lib/myclock.h"
#include "lib/pagetable.c"
#include "lib/sem.h"
#include "lib/shm.h"

// Shared Memory Globals
static int clock_id;
static struct my_clock* clock_shm;

static int page_table_id;
static struct page* page_table;

// For proteting the clock
static int clock_sem_id;

// For making memory references
static int mem_sem_ids[MAX_PROCS];

int main(int argc, char* argv[]) {
  srand(time(0));

  parse_command_options(argc, argv);

  setup_interrupt();

  clock_id = get_clock_shm();
  clock_shm = attach_to_clock_shm(clock_id);
  clock_shm->secs = 1;

  page_table_id = get_page_table();
  page_table = attach_to_page_table(page_table_id);

  setup_clock_sem();

  setup_mem_sems();

  pid_t children[MAX_PROCS];
  fork_and_exec_children(children);

  wait_for_all_children();

  free_shm();

  return EXIT_SUCCESS;
}

static void parse_command_options(int argc, char* argv[]) {
  int help_flag = 0;
  int c;

  while ((c = getopt(argc, argv, "h")) != -1) {
    switch (c) {
      case 'h':
        help_flag = 1;
        break;
      default:
        abort();
    }
  }

  if (help_flag) {
    print_help_message(argv[0]);
    exit(EXIT_SUCCESS);
  }
}

/**
 * Prints a help message.
 * The parameters correspond to program arguments.
 */
static void print_help_message(char* executable_name) {
  printf("Operating System Simulator\n\n");
  printf("Usage: ./%s\n\n", executable_name);
  printf("Arguments:\n");
  printf(" -h  Show help.\n");
}

/**
 * Free shared memory and abort program
 */
static void free_shm_and_abort(int signum) {
  free_shm();
  abort();
}

/**
 * Frees all allocated shared memory
 */
static void free_shm() {
  detach_from_clock_shm(clock_shm);
  shmctl(clock_id, IPC_RMID, 0);

  detach_from_page_table(page_table);
  shmctl(page_table_id, IPC_RMID, 0);

  deallocate_sem(clock_sem_id);

  deallocate_mem_sems();
}

/**
 * Set up the interrupt handler.
 */
static int setup_interrupt() {
  struct sigaction act;
  act.sa_handler = free_shm_and_abort;
  act.sa_flags = 0;
  int success = (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
  if (success == -1) {
    perror("Failed to set up handler for SIGPROF");
    return EXIT_FAILURE;
  }
  return success;
}

static void fork_and_exec_children(pid_t* children) {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    fork_and_exec_child(children, i);
  }
}

/**
 * Forks and execs a child process.
 * 
 * @param index Index of children PID array
 */
static void fork_and_exec_child(pid_t* children, int index) {
  children[index] = fork();

  if (children[index] == -1) {
    perror("Failed to fork");
    exit(EXIT_FAILURE);
  }

  if (children[index] == 0) {  // Child
    char pid_str[12];
    snprintf(pid_str,
             sizeof(pid_str),
             "%d",
             index);

    char clock_id_str[12];
    snprintf(clock_id_str,
             sizeof(clock_id_str),
             "%d",
             clock_id);

    char clock_sem_id_str[12];
    snprintf(clock_sem_id_str,
             sizeof(clock_sem_id_str),
             "%d",
             clock_sem_id);

    char mem_sem_id_str[12];
    snprintf(mem_sem_id_str,
             sizeof(mem_sem_id_str),
             "%d",
             mem_sem_ids[index]);

    execlp("user",
           "user",
           pid_str,
           clock_id_str,
           clock_sem_id_str,
           mem_sem_id_str,
           (char*) NULL);
    perror("Failed to exec");
    _exit(EXIT_FAILURE);
  }
}

/**
 * Allocates semaphores for
 * making memory references with
 * an initial value of 1.
 */
static void setup_mem_sems() {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    int sem_flags = IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR;
    mem_sem_ids[i] = allocate_sem(IPC_PRIVATE, sem_flags);
    init_sem(mem_sem_ids[i], 1);
  }
}

/**
 * Deallocate semaphores for
 * making memory references.
 */
static void deallocate_mem_sems() {
  int i = 0;
  for (; i < MAX_PROCS; i++) {
    deallocate_sem(mem_sem_ids[i]);
  }
}

/**
 * Allocates a semaphore for protecting
 * the logical clock with an initial
 * value of 1.
 */
static void setup_clock_sem() {
  int sem_flags = IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR;
  clock_sem_id = allocate_sem(IPC_PRIVATE, sem_flags);
  init_sem(clock_sem_id, 1);
}

static void wait_for_all_children() {
  pid_t pid;
  while ((pid = waitpid(-1, NULL, 0))) {
    if (errno == ECHILD) {
      break;
    }
  }
}