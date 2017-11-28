#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "user.h"
#include "lib/sem.h"
#include "lib/shm.h"

int main(int argc, char* argv[]) {
  srand(getpid());

  validate_number_of_args(argc);

  const int pid = atoi(argv[1]);
  const int clock_id = atoi(argv[2]);
  const int clock_sem_id = atoi(argv[3]);
  const int mem_ops_id = atoi(argv[4]);
  const int mem_sem_id = atoi(argv[5]);

  my_clock* clock_shm;
  clock_shm = attach_to_clock_shm(clock_id);

  mem_op_t* mem_ops;
  mem_ops = attach_to_mem_ops(mem_ops_id);

  update_clock_with_creation_time(clock_shm, clock_sem_id);

  int should_terminate = 0;
  int num_requests = 0;
  while (!should_terminate) {
    if (should_check_whether_to_terminate(num_requests)) {
      check_should_terminate(&should_terminate);
    }

    make_mem_request(mem_ops, pid);

    // Wait until request is granted
    sem_wait(mem_sem_id);
    num_requests++;
  }

  detach_from_clock_shm(clock_shm);
  detach_from_mem_ops(mem_ops);

  return EXIT_SUCCESS;
}

/**
 * Validates the program was passed
 * the correct number of arguments.
 *
 * Exits the program upon an invalid
 * number of arguments.
 * 
 * @param argc Argument count
 */
static void validate_number_of_args(int argc) {
  if (argc != ARGC) {
    fprintf(stderr, "Invalid number of arguments\n");
    exit(EXIT_FAILURE);
  }
}

static void update_clock_with_creation_time(my_clock* clock_shm,
                                            const int clock_sem_id) {
  sem_wait(clock_sem_id);
    unsigned int creation_time = get_creation_time();
    update_clock(clock_shm, creation_time);
  sem_post(clock_sem_id);
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

/**
 * Get a memory address to make a request to.
 * 
 * @return A memory address
 */
static unsigned int get_mem_addr() {
  return rand() % PROC_MEM;
}

/**
 * Get wheter the I/O operation
 * is a read or write.
 *
 * Reads are more likely than writes.
 * 
 * @return Read or write.
 */
static io_op get_read_or_write() {
  int rand_num = rand() % 3;
  if (rand_num != 0) {
    return READ;
  } else {
    return WRITE;
  }
}

/**
 * Check whether the program should terminate.
 * 
 * @param terminate_flag Set to 1 if should terminate. Else set to 0.
 */
static void check_should_terminate(int* terminate_flag) {
  int rand_num = rand() % 2;
  if (rand_num == 0) {
    *terminate_flag = 1;
  } else {
    *terminate_flag = 0;
  }
}

static void make_mem_request(mem_op_t* mem_ops, const int pid) {
  mem_ops[pid].addr = get_mem_addr();
  mem_ops[pid].op   = get_read_or_write();
}

static int should_check_whether_to_terminate(int num_requests) {
  if (num_requests != 0 && num_requests % 100 == 0) {
    return 1;
  } else {
    return 0;
  }
}