/**
 * Operating System Simulator
 *
 * Copyright (c) 2017 G Brenden Roques
 */

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include "oss.h"
#include "lib/myclock.h"
#include "lib/sem.h"
#include "lib/shm.h"

// Shared Memory Globals
static int clock_id;
static struct my_clock* clock_shm;

static int sem_id;  // For protecting the clock

int main(int argc, char* argv[]) {
  parse_command_options(argc, argv);

  setup_interrupt();

  clock_id = get_clock_shm();
  clock_shm = attach_to_clock_shm(clock_id);
  clock_shm->secs = 1;

  sem_id = allocate_sem(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
  init_sem(sem_id, 1);

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

  deallocate_sem(sem_id);
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