#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "user.h"
#include "lib/myclock.h"
#include "lib/sem.h"
#include "lib/shm.h"

int main(int argc, char* argv[]) {
  srand(time(0));

  validate_number_of_args(argc);

  const int pid = atoi(argv[1]);
  const int clock_id = atoi(argv[2]);
  const int clock_sem_id = atoi(argv[3]);
  const int mem_sem_id = atoi(argv[4]);

  struct my_clock* clock_shm;
  clock_shm = attach_to_clock_shm(clock_id);

  sem_wait(clock_sem_id);
    unsigned int creation_time = get_creation_time();
    update_clock(clock_shm, creation_time);
    fprintf(stderr, "PID %d running at %d:%d \n", pid, clock_shm->secs, clock_shm->nanosecs);
  sem_post(clock_sem_id);

  detach_from_clock_shm(clock_shm);

  return EXIT_SUCCESS;
}

static void validate_number_of_args(int argc) {
  if (argc != 5) {
    fprintf(stderr, "Invalid number of arguments\n");
    exit(EXIT_FAILURE);
  }
}

/**
 * Simulate the overhead of creating a new process.
 *
 * @return 1 - 500 milliseconds (in nanoseconds)
 */
static unsigned int get_creation_time() {
  int rand_num = rand() % 500 + 1;
  return (unsigned) rand_num * NANOSECS_PER_MILLISEC;
}

static void update_clock(struct my_clock* myclock, unsigned int nanosecs) {
  myclock->nanosecs += nanosecs;
  round_clock(myclock);
}