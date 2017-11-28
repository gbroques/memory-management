#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "shm.h"

/**
 * Allocates shared memory for a simulated clock.
 * 
 * @return The shared memory segment ID
 */
int get_clock_shm() {
  int id = shmget(IPC_PRIVATE, sizeof(my_clock),
    IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  if (id == -1) {
    perror("Failed to get shared memory for clock");
    exit(EXIT_FAILURE);
  }
  return id;
}

/**
 * Attaches to the clock shared memory segment.
 * 
 * @return A pointer to the clock in shared memory.
 */
my_clock* attach_to_clock_shm(int id) {
  void* clock_shm = shmat(id, NULL, 0);

  if (*((int*) clock_shm) == -1) {
    perror("Failed to attach to clock shared memory");
    exit(EXIT_FAILURE);
  }

  return (my_clock*) clock_shm;
}

/**
 * Detaches from clock shared memory.
 * 
 * @param Clock shared memory
 * @return On success, 0. On error -1.
 */
int detach_from_clock_shm(my_clock* shm) {
  int success = shmdt(shm);
  if (success == -1) {
    perror("Failed to detach from clock shared memory");
  }
  return success;
}

/**
 * Allocates shared memory for storing information
 * regarding memory operations for various processes.
 *
 * @param The number of processes
 * @return The shared memory segment ID
 */
int get_mem_ops(int num_procs) {
  int id = shmget(IPC_PRIVATE, sizeof(mem_op_t) * num_procs,
    IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

  if (id == -1) {
    perror("Failed to get shared memory for memory operations");
    exit(EXIT_FAILURE);
  }
  return id;
}

/**
 * Attaches to the shared memory for memory operations.
 * 
 * @return A pointer to the shared memory.
 */
mem_op_t* attach_to_mem_ops(int id) {
  void* mem_ops = shmat(id, NULL, 0);

  if (*((int*) mem_ops) == -1) {
    perror("Failed to attach to shared memory for memory operations");
    exit(EXIT_FAILURE);
  }

  return (mem_op_t*) mem_ops;
}

/**
 * Detaches from shared memory for memory operations.
 * 
 * @param A pointer to the shared memory.
 * @return On success, 0. On error -1.
 */
int detach_from_mem_ops(mem_op_t* shm) {
  int success = shmdt(shm);
  if (success == -1) {
    perror("Failed to detach from shared memory for memory operations");
  }
  return success;
}